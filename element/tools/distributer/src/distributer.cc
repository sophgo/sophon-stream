//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "distributer.h"

#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace distributer {
Distributer::Distributer() {}
Distributer::~Distributer() {}

common::ErrorCode Distributer::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    int _default_port =
        configure.find(CONFIG_INTERNAL_DEFAULT_PORT_FILED)->get<int>();
    default_port = _default_port;

    auto class_names_file =
        configure.find(CONFIG_INTERNAL_CLASS_NAMES_FILES_FILED)
            ->get<std::string>();
    std::ifstream istream;
    istream.open(class_names_file);
    assert(istream.is_open());
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length() - 1);
      class_names.push_back(line);
    }
    istream.close();

    auto rules = configure.find(CONFIG_INTERNAL_RULES_FILED);
    for (auto& rule : *rules) {
      int interval = rule.find(CONFIG_INTERNAL_INTERVAL_FILED)->get<float>();
      intervals.push_back(interval);
      last_times.push_back(0.0);

      auto routes = rule.find(CONFIG_INTERNAL_ROUTES_FILED);
      for (auto& route : *routes) {
        // 初始化当前interval下的每条路线
        int port_id = route.find(CONFIG_INTERNAL_PORT_FILED)->get<int>();
        std::vector<std::string> class_names_route =
            route.find(CONFIG_INTERNAL_CLASS_NAMES_FILED)
                ->get<std::vector<std::string>>();
        if (class_names_route.size() == 0) {
          // 没有配置class_names，则认为是full frame
          distrib_rules[interval]["full_frame"] = port_id;
        }
        for (auto& class_name : class_names_route) {
          distrib_rules[interval][class_name] = port_id;
        }
      }
    }
  } while (false);

  clocker.reset();

  return errorCode;
}

void Distributer::uninitInternal(){};

void Distributer::makeSubObjectMetadata(
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
    bm_image_format_info_t info;
    bm_status_t ret =
        bm_image_get_format_info(obj->mFrame->mSpData.get(), &info);
    auto image_format = info.image_format;
    auto data_format = info.data_type;
    ret = bm_image_create(obj->mFrame->mHandle, rect.crop_h, rect.crop_w,
                          image_format, data_format, cropped.get());
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

common::ErrorCode Distributer::doWork(int dataPipeId) {
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
      channel_id_internal % getOutputConnectorCapacity(default_port);
  errorCode = pushOutputData(default_port, outDataPipeId, data);

  // 判断计时器规则
  float cur_time = clocker.tell_ms() / 1000.0;
  int subId = 0;
  for (int i = 0; i < last_times.size(); ++i) {
    if (cur_time - last_times[i] > intervals[i]) {
      // 当前interval生效，更新时间
      last_times[i] = cur_time;
      // 按照distrib_rules[interval]记录的route分发
      for (auto detObj : objectMetadata->mDetectedObjectMetadatas) {
        // box分发
        int class_id = detObj->mClassify;
        std::string class_name = class_names[class_id];
        // 当前box需要发送到下游
        if (distrib_rules[intervals[i]].find(class_name) !=
            distrib_rules[intervals[i]].end()) {
          // 构造SubObjectMetadata
          std::shared_ptr<common::ObjectMetadata> subObj =
              std::make_shared<common::ObjectMetadata>();
          makeSubObjectMetadata(objectMetadata, detObj, subObj, subId);
          objectMetadata->mSubObjectMetadatas.push_back(subObj);
          ++objectMetadata->numBranches;
          // 发送subObj
          int target_port = distrib_rules[intervals[i]][class_name];
          int outDataPipeId =
              channel_id_internal % getOutputConnectorCapacity(target_port);
          errorCode = pushOutputData(target_port, outDataPipeId,
                                     std::static_pointer_cast<void>(subObj));
          if (common::ErrorCode::SUCCESS != errorCode) {
            IVS_WARN(
                "Send data fail, element id: {0:d}, output port: {1:d}, data: "
                "{2:p}",
                getId(), target_port, static_cast<void*>(subObj.get()));
          }
        }
        ++subId;
      }
      if (distrib_rules[intervals[i]].find("full_frame") !=
          distrib_rules[intervals[i]].end()) {
        // full_frame 分发，也是构造一个新的SubObjectMetadata
        std::shared_ptr<common::ObjectMetadata> subObj =
            std::make_shared<common::ObjectMetadata>();
        makeSubObjectMetadata(objectMetadata, nullptr, subObj, -1);
        objectMetadata->mSubObjectMetadatas.push_back(subObj);
        ++objectMetadata->numBranches;
        int target_port = distrib_rules[intervals[i]]["full_frame"];
        int outDataPipeId =
            channel_id_internal % getOutputConnectorCapacity(target_port);
        errorCode = pushOutputData(target_port, outDataPipeId,
                                   std::static_pointer_cast<void>(subObj));
      }
    }
  }
  return errorCode;
}

REGISTER_WORKER("distributer", Distributer)

}  // namespace distributer
}  // namespace element
}  // namespace sophon_stream