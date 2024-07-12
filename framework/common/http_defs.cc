//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "common/http_defs.h"

namespace sophon_stream {
namespace common {

void from_json(const nlohmann::json& j, Result& p) {
  if (j.count("Status")) p.status = j.at("Status").get<int>();
  if (j.count("TaskID")) p.taskId = j.at("TaskID").get<std::string>();
}

void to_json(nlohmann::json& j, const std::vector<Result>& p) {
  for (auto b : p)
    j.push_back(nlohmann::json{{"Status", b.status}, {"TaskID", b.taskId}});
}

void to_json(nlohmann::json& j, const Response& p) {
  j = nlohmann::json{{"Code", p.code}, {"Msg", p.msg}, {"Result", p.results}};
}
void from_json(const nlohmann::json& j, Response& p) {
  if (j.count("Code")) p.code = j.at("Code").get<int>();
  if (j.count("Msg")) p.msg = j.at("Msg").get<std::string>();
  if (j.count("Result")) p.results = j.at("Result").get<std::vector<Result>>();
}

void to_json(nlohmann::json& j, const RequestSingleInt& p) {
  j = nlohmann::json{{"idx", p.idx}};
}
void from_json(const nlohmann::json& j, RequestSingleInt& p) {
  if (j.count("idx")) p.idx = j.at("idx").get<int>();
}
bool str_to_object(const std::string& strjson, RequestSingleInt& request) {
  nlohmann::json json_object = nlohmann::json::parse(strjson);
  request = json_object;
  return true;
}

void to_json(nlohmann::json& j, const RequestSingleFloat& p) {
  j = nlohmann::json{{"value", p.value}};
}
void from_json(const nlohmann::json& j, RequestSingleFloat& p) {
  if (j.count("value")) p.value = j.at("value").get<float>();
}
bool str_to_object(const std::string& strjson, RequestSingleFloat& request) {
  nlohmann::json json_object = nlohmann::json::parse(strjson);
  request = json_object;
  return true;
}

}  // namespace common
}  // namespace sophon_stream