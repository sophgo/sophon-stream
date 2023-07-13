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
      line = line.substr(0, line.length() - 1);
      mClassNames.push_back(line);
    }
    istream.close();

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
          ++subId;
        }
      }
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