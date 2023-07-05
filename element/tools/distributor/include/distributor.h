//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_DISTRIBUTER_H_
#define SOPHON_STREAM_ELEMENT_DISTRIBUTER_H_

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "common/clocker.h"
#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace distributor {

class Distributor : public ::sophon_stream::framework::Element {
 public:
  Distributor();
  ~Distributor() override;

  common::ErrorCode initInternal(const std::string& json) override;
  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_RULES_FILED = "rules";
  static constexpr const char* CONFIG_INTERNAL_PORT_FILED = "port";
  static constexpr const char* CONFIG_INTERNAL_CLASS_NAMES_FILED = "classes";
  static constexpr const char* CONFIG_INTERNAL_DEFAULT_PORT_FILED =
      "default_port";
  static constexpr const char* CONFIG_INTERNAL_CLASS_NAMES_FILES_FILED =
      "class_names_file";
  static constexpr const char* CONFIG_INTERNAL_TIME_INTERVAL_FILED =
      "time_interval";
  static constexpr const char* CONFIG_INTERNAL_FRAME_INTERVAL_FILED =
      "frame_interval";
  static constexpr const char* CONFIG_INTERNAL_ROUTES_FILED = "routes";

 private:
  void makeSubObjectMetadata(
      std::shared_ptr<common::ObjectMetadata> obj,
      std::shared_ptr<common::DetectedObjectMetadata> detObj,
      std::shared_ptr<common::ObjectMetadata> subObj, int subId);
  /**
   * @brief 按时间间隔分发的所有规则。key：时间间隔，value：{类名，端口}
   */
  std::unordered_map<float, std::unordered_map<std::string, int>>
      mTimeDistribRules;

  /**
   * @brief 按帧间隔分发的所有规则。key：帧间隔，value：{类名，端口}
   */
  std::unordered_map<int, std::unordered_map<std::string, int>>
      mFrameDistribRules;

  std::vector<std::string> mClassNames;
  int mDefaultPort;
  std::vector<float> mTimeIntervals;
  std::vector<int> mFrameIntervals;
  std::vector<float> mLastTimes;

  sophon_stream::common::Clocker clocker;
};

}  // namespace distributor
}  // namespace element
}  // namespace sophon_stream

#endif