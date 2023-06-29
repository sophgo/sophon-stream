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

#include <unordered_map>
#include <unordered_set>

#include "common/clocker.h"
#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace distributer {

class Distributer : public ::sophon_stream::framework::Element {
 public:
  Distributer();
  ~Distributer() override;

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
  static constexpr const char* CONFIG_INTERNAL_INTERVAL_FILED = "interval";
  static constexpr const char* CONFIG_INTERNAL_ROUTES_FILED = "routes";

 private:
  void makeSubObjectMetadata(
      std::shared_ptr<common::ObjectMetadata> obj,
      std::shared_ptr<common::DetectedObjectMetadata> detObj,
      std::shared_ptr<common::ObjectMetadata> subObj, int subId);
  std::unordered_map<int, std::unordered_map<std::string, int>> distrib_rules;
  std::vector<std::string> class_names;
  int default_port;
  std::vector<float> intervals;
  std::vector<float> last_times;

  sophon_stream::common::Clocker clocker;
};

}  // namespace distributer
}  // namespace element
}  // namespace sophon_stream

#endif