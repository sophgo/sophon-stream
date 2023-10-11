//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "distributor.h"

#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace distributor {
Distributor::Distributor() {}
Distributor::~Distributor() {}

common::ErrorCode Distributor::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    int _default_port =
        configure.find(CONFIG_INTERNAL_DEFAULT_PORT_FILED)->get<int>();
    mDefaultPort = _default_port;

    auto class_names_file =
        configure.find(CONFIG_INTERNAL_CLASS_NAMES_FILES_FILED)
            ->get<std::string>();
    std::ifstream istream;
    istream.open(class_names_file);
    assert(istream.is_open());
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length());
      mClassNames.push_back(line);
    }
    istream.close();

    auto is_affineIt = configure.find(CONFIG_INTERNAL_IS_AFFINE_FIELD);
    if (is_affineIt != configure.end()) {
      is_affine = is_affineIt->get<bool>();
    } else {
      is_affine = false;
    }

    auto rules = configure.find(CONFIG_INTERNAL_RULES_FILED);
    for (auto& rule : *rules) {
      auto routes = rule.find(CONFIG_INTERNAL_ROUTES_FILED);
      auto time_interval_it = rule.find(CONFIG_INTERNAL_TIME_INTERVAL_FILED);
      if (time_interval_it != rule.end() && time_interval_it->is_number()) {
        float time_interval = time_interval_it->get<float>();
        mTimeIntervals.push_back(time_interval);
        for (auto& route : *routes) {
          // 初始化当前interval下的每条路线
          int port_id = route.find(CONFIG_INTERNAL_PORT_FILED)->get<int>();
          std::vector<std::string> class_names_route =
              route.find(CONFIG_INTERNAL_CLASS_NAMES_FILED)
                  ->get<std::vector<std::string>>();
          if (class_names_route.size() == 0) {
            // 没有配置class_names，则认为是full frame
            mTimeDistribRules[time_interval]["full_frame"] = port_id;
          }
          for (auto& class_name : class_names_route) {
            mTimeDistribRules[time_interval][class_name] = port_id;
          }
        }
      }
      auto frame_interval_it = rule.find(CONFIG_INTERNAL_FRAME_INTERVAL_FILED);
      if (frame_interval_it != rule.end() &&
          frame_interval_it->is_number_integer()) {
        int frame_interval = frame_interval_it->get<int>();
        mFrameIntervals.push_back(frame_interval);
        for (auto& route : *routes) {
          // 初始化当前interval下的每条路线
          int port_id = route.find(CONFIG_INTERNAL_PORT_FILED)->get<int>();
          std::vector<std::string> class_names_route =
              route.find(CONFIG_INTERNAL_CLASS_NAMES_FILED)
                  ->get<std::vector<std::string>>();
          if (class_names_route.size() == 0) {
            // 没有配置class_names，则认为是full frame
            mFrameDistribRules[frame_interval]["full_frame"] = port_id;
          }
          for (auto& class_name : class_names_route) {
            mFrameDistribRules[frame_interval][class_name] = port_id;
          }
        }
      }
      if (time_interval_it == rule.end() && frame_interval_it == rule.end()) {
        // 用户不配置time_interval和frame_interval，则视为对每一帧做分发，放在frame_interval相关规则里
        int curFrameInterval = 1;
        mFrameIntervals.push_back(curFrameInterval);
        for (auto& route : *routes) {
          // 初始化当前interval下的每条路线
          int port_id = route.find(CONFIG_INTERNAL_PORT_FILED)->get<int>();
          std::vector<std::string> class_names_route =
              route.find(CONFIG_INTERNAL_CLASS_NAMES_FILED)
                  ->get<std::vector<std::string>>();
          if (class_names_route.size() == 0) {
            // 没有配置class_names，则认为是full frame
            mFrameDistribRules[curFrameInterval]["full_frame"] = port_id;
          }
          for (auto& class_name : class_names_route) {
            mFrameDistribRules[curFrameInterval][class_name] = port_id;
          }
        }
      }
    }
    if (mTimeIntervals.size() > 1) {
      std::sort(mTimeIntervals.begin(), mTimeIntervals.end());
      mTimeIntervals.erase(
          std::unique(mTimeIntervals.begin(), mTimeIntervals.end()),
          mTimeIntervals.end());
    }
    // mLastTimes = std::vector<float>(mTimeIntervals.size(), -99.0);
    if (mFrameIntervals.size() > 1) {
      std::sort(mFrameIntervals.begin(), mFrameIntervals.end());
      mFrameIntervals.erase(
          std::unique(mFrameIntervals.begin(), mFrameIntervals.end()),
          mFrameIntervals.end());
    }

  } while (false);

  clocker.reset();

  return errorCode;
}

void Distributor::makeSubObjectMetadata(
    std::shared_ptr<common::ObjectMetadata> obj,
    std::shared_ptr<common::DetectedObjectMetadata> detObj,
    std::shared_ptr<common::ObjectMetadata> subObj, int subId) {
  bmcv_rect_t rect;
  if (detObj != nullptr) {
    rect.start_x = detObj->mBox.mX;
    rect.start_y = detObj->mBox.mY;
    rect.crop_w = detObj->mBox.mWidth;
    rect.crop_h = detObj->mBox.mHeight;
  }

  subObj->mFrame = std::make_shared<common::Frame>();

  // crop or not
  if (detObj != nullptr) {
    std::shared_ptr<bm_image> cropped = nullptr;
    cropped.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    bm_status_t ret =
        bm_image_create(obj->mFrame->mHandle, rect.crop_h, rect.crop_w,
                        obj->mFrame->mSpData->image_format,
                        obj->mFrame->mSpData->data_type, cropped.get());
    ret = bmcv_image_crop(obj->mFrame->mHandle, 1, &rect, *obj->mFrame->mSpData,
                          cropped.get());

    subObj->mFrame->mSpData = cropped;
  } else {
    subObj->mFrame->mSpData = obj->mFrame->mSpData;
  }

  // update frameid, channelid
  subObj->mFrame->mFrameId = obj->mFrame->mFrameId;
  subObj->mFrame->mChannelId = obj->mFrame->mChannelId;
  subObj->mFrame->mChannelIdInternal = obj->mFrame->mChannelIdInternal;
  subObj->mSubId = subId;
}

cv::Mat Distributor::estimateAffine2D(
    const std::vector<cv::Point2f>& src_points,
    const std::vector<cv::Point2f>& dst_points) {
  if (src_points.size() != dst_points.size() || src_points.size() < 3) {
    std::cerr << "Error: Invalid input points." << std::endl;
    return cv::Mat();
  }

  cv::Mat A(src_points.size() * 2, 6, CV_32F);
  cv::Mat B(src_points.size() * 2, 1, CV_32F);

  for (size_t i = 0; i < src_points.size(); ++i) {
    float x = src_points[i].x;
    float y = src_points[i].y;
    float u = dst_points[i].x;
    float v = dst_points[i].y;

    A.at<float>(i * 2, 0) = x;
    A.at<float>(i * 2, 1) = y;
    A.at<float>(i * 2, 2) = 1;
    A.at<float>(i * 2, 3) = 0;
    A.at<float>(i * 2, 4) = 0;
    A.at<float>(i * 2, 5) = 0;

    A.at<float>(i * 2 + 1, 0) = 0;
    A.at<float>(i * 2 + 1, 1) = 0;
    A.at<float>(i * 2 + 1, 2) = 0;
    A.at<float>(i * 2 + 1, 3) = x;
    A.at<float>(i * 2 + 1, 4) = y;
    A.at<float>(i * 2 + 1, 5) = 1;

    B.at<float>(i * 2) = u;
    B.at<float>(i * 2 + 1) = v;
  }

  cv::Mat X;
  cv::solve(A, B, X, cv::DECOMP_SVD);

  cv::Mat M = (cv::Mat_<float>(2, 3) << X.at<float>(0), X.at<float>(1),
               X.at<float>(2), X.at<float>(3), X.at<float>(4), X.at<float>(5));

  return M.clone();
}

void Distributor::makeSubFaceObjectMetadata(
    std::shared_ptr<common::ObjectMetadata> obj,
    std::shared_ptr<common::FaceObjectMetadata> faceObj,
    std::shared_ptr<common::ObjectMetadata> subObj, int subId) {
  bmcv_rect_t rect;
  if (faceObj != nullptr) {
    rect.start_x = faceObj->left;
    rect.start_y = faceObj->top;
    rect.crop_w = faceObj->right - faceObj->left + 1;
    rect.crop_h = faceObj->bottom - faceObj->top + 1;
  }
  subObj->mFrame = std::make_shared<common::Frame>();
  // crop or not,faceObj != nullptr
  if (faceObj != nullptr) {
    int x1 = faceObj->left;
    int y1 = faceObj->top;
    int x2 = faceObj->right;
    int y2 = faceObj->bottom;
    bool is_affine = false;

    if (is_affine) {
      std::vector<cv::Point2f> key_loc_ref = {
          cv::Point2f(30.2946, 51.6963), cv::Point2f(65.5318, 51.6963),
          cv::Point2f(48.0252, 71.7366), cv::Point2f(33.5493, 92.3655),
          cv::Point2f(62.7299, 92.3655)};
      // 外扩100%，防止对齐后人脸出现黑边
      int new_x1 = std::max(int(1.50 * x1 - 0.50 * x2), 0);
      int new_x2 =
          std::min(int(1.50 * x2 - 0.50 * x1), obj->mFrame->mWidth - 1);
      int new_y1 = std::max(int(1.50 * y1 - 0.50 * y2), 0);
      int new_y2 =
          std::min(int(1.50 * y2 - 0.50 * y1), obj->mFrame->mWidth - 1);

      rect.start_x = new_x1;
      rect.start_y = new_y1;
      rect.crop_w = new_x2 - new_x1;
      rect.crop_h = new_y2 - new_y1;

      bm_image corp_img;
      bm_status_t ret =
          bm_image_create(obj->mFrame->mHandle, rect.crop_h, rect.crop_w,
                          obj->mFrame->mSpData->image_format,
                          obj->mFrame->mSpData->data_type, &corp_img);
      ret = bmcv_image_crop(obj->mFrame->mHandle, 1, &rect,
                            *obj->mFrame->mSpData, &corp_img);

      // 得到原始图中关键点
      float left_eye_x = faceObj->points_x[0];
      float left_eye_y = faceObj->points_y[0];
      float right_eye_x = faceObj->points_x[1];
      float right_eye_y = faceObj->points_y[1];
      float nose_x = faceObj->points_x[2];
      float nose_y = faceObj->points_y[2];
      float left_mouth_x = faceObj->points_x[3];
      float left_mouth_y = faceObj->points_y[3];
      float right_mouth_x = faceObj->points_x[4];
      float right_mouth_y = faceObj->points_y[4];

      // 得到外扩100% 后图中关键点坐标(外扩的目的是为了防止对齐后出现黑边)
      float new_left_eye_x = left_eye_x - new_x1;
      float new_right_eye_x = right_eye_x - new_x1;
      float new_nose_x = nose_x - new_x1;
      float new_left_mouth_x = left_mouth_x - new_x1;
      float new_right_mouth_x = right_mouth_x - new_x1;
      float new_left_eye_y = left_eye_y - new_y1;
      float new_right_eye_y = right_eye_y - new_y1;
      float new_nose_y = nose_y - new_y1;
      float new_left_mouth_y = left_mouth_y - new_y1;
      float new_right_mouth_y = right_mouth_y - new_y1;

      double enlargeFace_landmark[10] = {new_left_eye_x,    new_right_eye_x,
                                         new_nose_x,        new_left_mouth_x,
                                         new_right_mouth_x, new_left_eye_y,
                                         new_right_eye_y,   new_nose_y,
                                         new_left_mouth_y,  new_right_mouth_y};

      std::vector<cv::Point2f> key_loc = {
          cv::Point2f(new_left_eye_x, new_left_eye_y),
          cv::Point2f(new_right_eye_x, new_right_eye_y),
          cv::Point2f(new_nose_x, new_nose_y),
          cv::Point2f(new_left_mouth_x, new_left_mouth_y),
          cv::Point2f(new_right_mouth_x, new_right_mouth_y)};

      cv::Mat affine_matrix = estimateAffine2D(key_loc, key_loc_ref);

      bmcv_warp_matrix warp_matrix;
      bmcv_affine_image_matrix matrix = {&warp_matrix, 1};

      matrix.matrix->m[0] = affine_matrix.at<float>(0, 0);
      matrix.matrix->m[1] = affine_matrix.at<float>(0, 1);
      matrix.matrix->m[2] = affine_matrix.at<float>(0, 2);
      matrix.matrix->m[3] = affine_matrix.at<float>(1, 0);
      matrix.matrix->m[4] = affine_matrix.at<float>(1, 1);
      matrix.matrix->m[5] = affine_matrix.at<float>(1, 2);

      std::shared_ptr<bm_image> affine_image_ptr = nullptr;
      affine_image_ptr.reset(new bm_image, [](bm_image* p) {
        bm_image_destroy(*p);
        delete p;
        p = nullptr;
      });

      bm_image planar_image;
      ret = bm_image_create(obj->mFrame->mHandle, corp_img.height,
                            corp_img.width, FORMAT_BGR_PLANAR,
                            DATA_TYPE_EXT_1N_BYTE, &planar_image);
      ret = bmcv_image_storage_convert(obj->mFrame->mHandle, 1, &corp_img,
                                       &planar_image);

      ret = bm_image_create(obj->mFrame->mHandle, planar_image.height,
                            planar_image.width, planar_image.image_format,
                            planar_image.data_type, affine_image_ptr.get());
      ret = bmcv_image_warp_affine_similar_to_opencv(obj->mFrame->mHandle, 1,
                                                     &matrix, &planar_image,
                                                     affine_image_ptr.get(), 0);

      subObj->mFrame->mSpData = affine_image_ptr;
      // subObj->mFrame->mSpData = obj->mFrame->mSpData;
      bm_image_destroy(planar_image);
      bm_image_destroy(corp_img);
    } else {
      rect.start_x = std::max(x1, 0);
      rect.start_y = std::max(y1, 0);
      rect.crop_w = std::max(x2 - x1 + 1, 0);
      rect.crop_h = std::max(y2 - y1 + 1, 0);

      std::shared_ptr<bm_image> cropped = nullptr;
      cropped.reset(new bm_image, [](bm_image* p) {
        bm_image_destroy(*p);
        delete p;
        p = nullptr;
      });
      bm_status_t ret = bm_image_create(obj->mFrame->mHandle, rect.crop_h,
                                        rect.crop_w, FORMAT_BGR_PLANAR,
                                        DATA_TYPE_EXT_1N_BYTE, cropped.get());
      ret = bmcv_image_crop(obj->mFrame->mHandle, 1, &rect,
                            *obj->mFrame->mSpData, cropped.get());
      subObj->mFrame->mSpData = obj->mFrame->mSpData;
    }

  } else {
    subObj->mFrame->mSpData = obj->mFrame->mSpData;
  }

  bm_image image = *subObj->mFrame->mSpData;

  // update frameid, channelid
  subObj->mFrame->mFrameId = obj->mFrame->mFrameId;
  subObj->mFrame->mChannelId = obj->mFrame->mChannelId;
  subObj->mFrame->mChannelIdInternal = obj->mFrame->mChannelIdInternal;
  subObj->mSubId = subId;
}

common::ErrorCode Distributor::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  // 从队列中取出一个数据，判断detection结果，如果需要发送到下游，则做crop并且发送
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  // 先把ObjectMetadata发给默认的汇聚节点
  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      channel_id_internal % getOutputConnectorCapacity(mDefaultPort);

  if (mChannelLastTimes.find(channel_id_internal) == mChannelLastTimes.end()) {
    mChannelLastTimes[channel_id_internal] =
        std::vector<float>(mTimeIntervals.size(), -99.0);
  }
  // 判断计时器规则
  float cur_time = clocker.tell_ms() / 1000.0;
  int subId = 0;
  std::unordered_map<std::string, std::unordered_set<int>> class2ports;
  for (int i = 0; i < mChannelLastTimes[channel_id_internal].size(); ++i) {
    if (cur_time - mChannelLastTimes[channel_id_internal][i] >
        mTimeIntervals[i]) {
      // IVS_DEBUG("meet time interval rules, frame id = {0}",
      // objectMetadata->mFrame->mFrameId);
      mChannelLastTimes[channel_id_internal][i] = cur_time;
      for (auto class_port_it = mTimeDistribRules[mTimeIntervals[i]].begin();
           class_port_it != mTimeDistribRules[mTimeIntervals[i]].end();
           ++class_port_it) {
        class2ports[class_port_it->first].insert(class_port_it->second);
      }
    }
  }
  // 判断跳帧规则
  for (int i = 0; i < mFrameIntervals.size(); ++i) {
    if (objectMetadata->mFrame->mFrameId % mFrameIntervals[i] == 0) {
      for (auto class_port_it = mFrameDistribRules[mFrameIntervals[i]].begin();
           class_port_it != mFrameDistribRules[mFrameIntervals[i]].end();
           ++class_port_it) {
        class2ports[class_port_it->first].insert(class_port_it->second);
      }
    }
  }

  if (class2ports.size() > 0) {
    for (auto faceObj : objectMetadata->mFaceObjectMetadatas) {
      int class_id = 0;
      std::string class_name = mClassNames[class_id];
      if (class2ports.find(class_name) != class2ports.end()) {
        for (auto port_it = class2ports[class_name].begin();
             port_it != class2ports[class_name].end(); ++port_it) {
          int target_port = *port_it;
          // 构造SubObjectMetadata
          std::shared_ptr<common::ObjectMetadata> subObj =
              std::make_shared<common::ObjectMetadata>();
          makeSubFaceObjectMetadata(objectMetadata, faceObj, subObj, subId);
          objectMetadata->mSubObjectMetadatas.push_back(subObj);
          ++objectMetadata->numBranches;

          int outDataPipeId =
              channel_id_internal % getOutputConnectorCapacity(target_port);
          errorCode = pushOutputData(target_port, outDataPipeId,
                                     std::static_pointer_cast<void>(subObj));
          IVS_DEBUG(
              "Sub ObjectMetadata is sent to branch, channel_id = {0}, "
              "frame_id = {1}, subId = {2}",
              channel_id_internal, subObj->mFrame->mFrameId, subId);
          if (common::ErrorCode::SUCCESS != errorCode) {
            IVS_WARN(
                "Send data fail, element id: {0:d}, output port: {1:d}, data: "
                "{2:p}",
                getId(), target_port, static_cast<void*>(subObj.get()));
          }
        }
      }
      ++subId;
    }

    for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
      int class_id = detObj->mClassify;
      std::string class_name = mClassNames[class_id];
      if (class2ports.find(class_name) != class2ports.end()) {
        for (auto port_it = class2ports[class_name].begin();
             port_it != class2ports[class_name].end(); ++port_it) {
          int target_port = *port_it;
          // 构造SubObjectMetadata
          std::shared_ptr<common::ObjectMetadata> subObj =
              std::make_shared<common::ObjectMetadata>();
          makeSubObjectMetadata(objectMetadata, detObj, subObj, subId);
          objectMetadata->mSubObjectMetadatas.push_back(subObj);
          ++objectMetadata->numBranches;
          int outDataPipeId =
              channel_id_internal % getOutputConnectorCapacity(target_port);
          errorCode = pushOutputData(target_port, outDataPipeId,
                                     std::static_pointer_cast<void>(subObj));
          IVS_DEBUG(
              "Sub ObjectMetadata is sent to branch, channel_id = {0}, "
              "frame_id = {1}, subId = {2}",
              channel_id_internal, subObj->mFrame->mFrameId, subId);
          if (common::ErrorCode::SUCCESS != errorCode) {
            IVS_WARN(
                "Send data fail, element id: {0:d}, output port: {1:d}, data: "
                "{2:p}",
                getId(), target_port, static_cast<void*>(subObj.get()));
          }   
        }
      }
      ++subId;
    }


    if (class2ports.find("full_frame") != class2ports.end()) {
      for (auto port_it = class2ports["full_frame"].begin();
           port_it != class2ports["full_frame"].end(); ++port_it) {
        // full_frame 分发，也是构造一个新的SubObjectMetadata
        std::shared_ptr<common::ObjectMetadata> subObj =
            std::make_shared<common::ObjectMetadata>();
        makeSubObjectMetadata(objectMetadata, nullptr, subObj, -1);
        objectMetadata->mSubObjectMetadatas.push_back(subObj);
        ++objectMetadata->numBranches;
        int target_port = *port_it;
        int outDataPipeId =
            channel_id_internal % getOutputConnectorCapacity(target_port);
        errorCode = pushOutputData(target_port, outDataPipeId,
                                   std::static_pointer_cast<void>(subObj));
      }
    }
  }

  errorCode = pushOutputData(mDefaultPort, outDataPipeId, data);
  IVS_DEBUG(
      "Main ObjectMetadata is sent to Converger, channel_id = {0}, frame_id = "
      "{1}, numBranches = {2}",
      channel_id_internal, objectMetadata->mFrame->mFrameId,
      objectMetadata->numBranches);

  return errorCode;
}

REGISTER_WORKER("distributor", Distributor)

}  // namespace distributor
}  // namespace element
}  // namespace sophon_stream