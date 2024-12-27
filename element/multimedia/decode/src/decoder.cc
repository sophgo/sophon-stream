//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "decoder.h"

namespace sophon_stream {
namespace element {
namespace decode {

// camera synchronization
std::mutex Decoder::decoder_mutex;
std::condition_variable Decoder::decoder_cv;
int Decoder::numThreadsReady = 0;
std::atomic<int> Decoder::numThreadsTotal(0);

bool check_path(std::string file_path,
                std::vector<std::string> correct_postfixes) {
  auto index = file_path.rfind('.');
  std::string postfix = file_path.substr(index + 1);
  if (find(correct_postfixes.begin(), correct_postfixes.end(), postfix) !=
      correct_postfixes.end()) {
    return true;
  } else {
    IVS_ERROR("skipping path: {0},please check your dataset!", file_path);
    return false;
  }
};

void getAllFiles(std::string path, std::vector<std::string>& files,
                 std::vector<std::string> correct_postfixes) {
  DIR* dir;
  struct dirent* ptr;
  if ((dir = opendir(path.c_str())) == NULL) {
    perror("Open dri error...");
    exit(1);
  }
  while ((ptr = readdir(dir)) != NULL) {
    if (std::strcmp(ptr->d_name, ".") == 0 ||
        std::strcmp(ptr->d_name, "..") == 0)
      continue;
    else if (ptr->d_type == 8 &&
             check_path(path + "/" + ptr->d_name, correct_postfixes))  // file
      files.push_back(path + "/" + ptr->d_name);
    else if (ptr->d_type == 10)  // link file
      continue;
    else if (ptr->d_type == 4) {
      // files.push_back(ptr->d_name);//dir
      getAllFiles(path + "/" + ptr->d_name, files, correct_postfixes);
    }
  }
  closedir(dir);
}

void bm_image2Frame(std::shared_ptr<common::Frame>& f, bm_image& img) {
  f->mWidth = img.width;
  f->mHeight = img.height;
  f->mDataType = img.data_type;
  f->mFormatType = img.image_format;
  f->mChannel = 3;
  f->mDataSize = img.width * img.height * f->mChannel * sizeof(uchar);
}

Decoder::Decoder() {
  // 获取线程数
  numThreadsTotal.fetch_add(1);
}

Decoder::~Decoder() {
  numThreadsTotal.fetch_sub(1);
  decoder_cv.notify_all();
  // bm_dev_free(m_handle);
}

common::ErrorCode Decoder::init(int graphId,
                                const ChannelOperateRequest& request,
                                bm_handle_t handle_) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    mUrl = request.url;
    mLoopNum = request.loopNum;
    mSampleInterval = request.sampleInterval;
    mFps = request.fps;
    mSampleStrategy = request.sampleStrategy;
    // int ret = bm_dev_request(&m_handle, deviceId);
    m_handle = handle_;
    mDeviceId = bm_get_devid(m_handle);
    mGraphId = graphId;
    mSourceType = request.sourceType;
    mImgIndex = 0;
    mRoiPredefined = request.roi_predefined;
    if (mRoiPredefined) {
      mRoi.start_x = request.roi.start_x;
      mRoi.start_y = request.roi.start_y;
      mRoi.crop_w = request.roi.crop_w;
      mRoi.crop_h = request.roi.crop_h;
    }

    if (mSourceType == ChannelOperateRequest::SourceType::VIDEO) {
      decoder.mFrameCount(mUrl.c_str(), mFrameCount);
      if (!mFrameCount) {
        IVS_ERROR("Decoder::init error, mFrameCount: {0}", mFrameCount);
      }
      IVS_INFO("Decoder::init, mFrameCount: {0}", mFrameCount);
    }

    if (mSourceType == ChannelOperateRequest::SourceType::IMG_DIR) {
      std::vector<std::string> correct_postfixes = {"jpg", "JPEG", "png",
                                                    "bmp"};
      getAllFiles(mUrl, mImagePaths, correct_postfixes);
      std::sort(mImagePaths.begin(), mImagePaths.end());
      decoder.setFps(mFps);
    }

    if (mSourceType == ChannelOperateRequest::SourceType::BASE64) {
      mgr = HTTP_Base64_Mgr::GetInstance();
      mgr->init(request.base64Port, mUrl);
      mgr->setFps(mFps);
      IVS_DEBUG("Decoder::init, base64Port: {0}", request.base64Port);
    }

    if (mSourceType == ChannelOperateRequest::SourceType::RTSP ||
        mSourceType == ChannelOperateRequest::SourceType::RTMP ||
        mSourceType == ChannelOperateRequest::SourceType::GB28181 ||
        mSourceType == ChannelOperateRequest::SourceType::CAMERA ||
        mSourceType == ChannelOperateRequest::SourceType::VIDEO) {
      decoder.setFps(mFps);
      auto ret = decoder.openDec(&m_handle, mUrl.c_str());
      if (ret < 0) {
        IVS_ERROR(
            "Decoder::init error, openDec failed, ret: {0}, channel id : {1}",
            ret, request.channelId);
        errorCode = common::ErrorCode::ERR_FFMPEG_INPUT_CTX_OPEN;
        break;
      }
    }

  } while (false);

  return errorCode;
}

common::ErrorCode Decoder::process(
    std::shared_ptr<common::ObjectMetadata>& objectMetadata) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  if (mSourceType == ChannelOperateRequest::SourceType::RTSP ||
      mSourceType == ChannelOperateRequest::SourceType::RTMP ||
      mSourceType == ChannelOperateRequest::SourceType::GB28181) {
    int frame_id = 0;
    int eof = 0;
    std::shared_ptr<bm_image> spBmImage = nullptr;
    int64_t pts = 0;
    spBmImage =
        decoder.grab(frame_id, eof, pts, mSampleInterval, mSampleStrategy);
    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = frame_id;
    objectMetadata->mFrame->mSubFrameIdVec.push_back(frame_id);
    objectMetadata->mFrame->mSpData = spBmImage;
    objectMetadata->mFrame->mTimestamp = pts;
    objectMetadata->mGraphId = mGraphId;
    if (eof) {
      objectMetadata->mFrame->mEndOfStream = true;
      errorCode = common::ErrorCode::STREAM_END;
    } else {
      if (spBmImage != nullptr)
        bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    }

    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  } else if (mSourceType == ChannelOperateRequest::SourceType::VIDEO) {
    int frame_id = 0;
    int eof = 0;
    std::shared_ptr<bm_image> spBmImage = nullptr;
    int64_t pts = 0;
    spBmImage =
        decoder.grab(frame_id, eof, pts, mSampleInterval, mSampleStrategy);
    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = frame_id;
    objectMetadata->mFrame->mSubFrameIdVec.push_back(frame_id);
    objectMetadata->mFrame->mSpData = spBmImage;
    objectMetadata->mFrame->mTimestamp = pts;
    objectMetadata->mGraphId = mGraphId;
    /* 当mLoopNum > 1，在最后一帧初始化decoder，开始下一个循环 */
    if (mLoopNum > 1 && (mImgIndex++ == mFrameCount - 1)) {
      --mLoopNum;
      mImgIndex = 0;
      decoder.closeDec();
      decoder.openDec(&m_handle, mUrl.c_str());
    }

    if (eof) {
      objectMetadata->mFrame->mEndOfStream = true;
      errorCode = common::ErrorCode::STREAM_END;
    } else {
      if (spBmImage != nullptr)
        bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    }

    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  } else if (mSourceType == ChannelOperateRequest::SourceType::IMG_DIR) {
    std::shared_ptr<bm_image> spBmImage = nullptr;

    spBmImage = decoder.picDec(
        m_handle, mImagePaths[mImgIndex % mImagePaths.size()].c_str());
    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = mImgIndex;
    objectMetadata->mFrame->mSubFrameIdVec.push_back(mImgIndex);
    objectMetadata->mFrame->mSpData = spBmImage;
    objectMetadata->mGraphId = mGraphId;

    if (!mLoopNum) {
      objectMetadata->mFrame->mEndOfStream = true;
      errorCode = common::ErrorCode::STREAM_END;
    } else {
      if (spBmImage != nullptr)
        bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    }

    /* mImgIndex会不停累加，mImgIndex % mImagePaths.size()的值为
    mImagePaths.size() - 1，即最后一张图像时，mLoopNum-1 */
    if ((mImgIndex % mImagePaths.size()) == (mImagePaths.size() - 1))
      --mLoopNum;
    ++mImgIndex;

    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  } else if (mSourceType == ChannelOperateRequest::SourceType::BASE64) {
    std::shared_ptr<bm_image> spBmImage = nullptr;

    spBmImage = mgr->grab(m_handle);
    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = mImgIndex++;
    objectMetadata->mFrame->mSubFrameIdVec.push_back(mImgIndex);
    objectMetadata->mFrame->mSpData = spBmImage;
    objectMetadata->mGraphId = mGraphId;
    if (spBmImage != nullptr)
      bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  } else if (mSourceType == ChannelOperateRequest::SourceType::CAMERA) {
    int frame_id = 0;
    int eof = 0;
    std::shared_ptr<bm_image> spBmImage = nullptr;
    int64_t pts = 0;

   {  // 在所有线程等待执行decoder.grab前添加一个等待点
      std::unique_lock<std::mutex> lock(decoder_mutex);
      numThreadsReady ++;
      if(numThreadsReady >= numThreadsTotal){
        numThreadsReady = 0;
        lock.unlock();
        decoder_cv.notify_all();
      }else{
        decoder_cv.wait(lock);
      }
    }
    spBmImage =
        decoder.grab(frame_id, eof, pts, mSampleInterval, mSampleStrategy);
   

    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = frame_id;
    objectMetadata->mFrame->mSubFrameIdVec.push_back(frame_id);
    objectMetadata->mFrame->mSpData = spBmImage;
    objectMetadata->mFrame->mTimestamp = pts;
    objectMetadata->mGraphId = mGraphId;

    if (eof) {
      objectMetadata->mFrame->mEndOfStream = true;
      errorCode = common::ErrorCode::STREAM_END;
    } else {
      if (spBmImage != nullptr)
        bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    }

    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  }
  objectMetadata->mFilter =
      (objectMetadata->mFrame->mFrameId % mSampleInterval != 0) ? true : false;

  // if (objectMetadata->mFilter) printf("%d filter \n",
  // objectMetadata->mFrame->mFrameId); else printf("%d keep \n",
  // objectMetadata->mFrame->mFrameId);

  if (objectMetadata->mFrame->mSpData && mRoiPredefined) {
    std::shared_ptr<bm_image> cropped = nullptr;
    cropped.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    bm_status_t ret = bm_image_create(
        objectMetadata->mFrame->mHandle, mRoi.crop_h, mRoi.crop_w,
        objectMetadata->mFrame->mSpData->image_format,
        objectMetadata->mFrame->mSpData->data_type, cropped.get());

    // #if BMCV_VERSION_MAJOR > 1
    //     ret = bmcv_image_vpp_convert(objectMetadata->mFrame->mHandle, 1,
    //     *objectMetadata->mFrame->mSpData,
    //                                 cropped.get(), &mRoi);
    // #else
    ret = bmcv_image_crop(objectMetadata->mFrame->mHandle, 1, &mRoi,
                          *objectMetadata->mFrame->mSpData, cropped.get());
    // #endif
    if (!ret) {
      bm_image2Frame(objectMetadata->mFrame, *cropped);
      objectMetadata->mFrame->mSpData = cropped;
    }

    else
      IVS_ERROR("Decoder roi unreasonable");
  }

  return errorCode;
}

void Decoder::uninit() {}

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream