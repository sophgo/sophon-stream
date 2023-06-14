#include "decoder.h"

#include <nlohmann/json.hpp>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace decode {

std::vector<std::string> stringSplit(const std::string& str, char delim) {
  std::string s;
  s.append(1, delim);
  std::regex reg(s);
  std::vector<std::string> elems(
      std::sregex_token_iterator(str.begin(), str.end(), reg, -1),
      std::sregex_token_iterator());
  return elems;
}

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

Decoder::Decoder() {}

Decoder::~Decoder() {}

common::ErrorCode Decoder::init(
    int deviceId, const std::string& url,
    const ChannelOperateRequest::SourceType& sourceType) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    mUrl = url;

    int ret = bm_dev_request(&m_handle, deviceId);
    mDeviceId = deviceId;
    mSourceType = sourceType;
    assert(BM_SUCCESS == ret);

    decoder.openDec(&m_handle, mUrl.c_str());

    if (mSourceType == ChannelOperateRequest::SourceType::IMG_DIR) {
      mImgIndex = 0;
      std::vector<std::string> correct_postfixes = {"jpg", "png", "bmp"};
      getAllFiles(mUrl, mImagePaths, correct_postfixes);
      std::sort(mImagePaths.begin(), mImagePaths.end());
    }
  } while (false);

  return errorCode;
}

common::ErrorCode Decoder::process(
    std::shared_ptr<common::ObjectMetadata>& objectMetadata) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  if (mSourceType == ChannelOperateRequest::SourceType::RTSP ||
      mSourceType == ChannelOperateRequest::SourceType::RTMP ||
      mSourceType == ChannelOperateRequest::SourceType::VIDEO) {
    int frame_id = 0;
    int eof = 0;
    std::shared_ptr<bm_image> spBmImage = decoder.grab(frame_id, eof);

    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();

    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = frame_id;
    objectMetadata->mFrame->mSpData = spBmImage;
    if (eof) {
      objectMetadata->mFrame->mEndOfStream = true;
      errorCode = common::ErrorCode::STREAM_END;
    } else {
      bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    }

    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  } else if (mSourceType == ChannelOperateRequest::SourceType::IMG_DIR) {
    std::shared_ptr<bm_image> spBmImage = nullptr;

    if (mImgIndex < mImagePaths.size()) {
      spBmImage = picDec(m_handle, mImagePaths[mImgIndex].c_str());
    }
    ++mImgIndex;
    objectMetadata = std::make_shared<common::ObjectMetadata>();
    objectMetadata->mFrame = std::make_shared<common::Frame>();
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mFrameId = mImgIndex;
    objectMetadata->mFrame->mSpData = spBmImage;

    if (mImgIndex >= mImagePaths.size()) {
      objectMetadata->mFrame->mEndOfStream = true;
      errorCode = common::ErrorCode::STREAM_END;
    } else {
      bm_image2Frame(objectMetadata->mFrame, *spBmImage);
    }

    if (common::ErrorCode::SUCCESS != errorCode) {
      objectMetadata->mErrorCode = errorCode;
    }
  }

  return errorCode;
}

void Decoder::uninit() {}

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream