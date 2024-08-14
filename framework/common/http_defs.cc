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

void to_json(nlohmann::json& j, const RequestAddChannel& p) {
  j = nlohmann::json{
      {"channel_id", p.channel_id},   {"url", p.url},
      {"source_type", p.source_type}, {"sample_interval", p.sample_interval},
      {"decode_id", p.decode_id},     {"fps", p.fps},
      {"loop_num", p.loop_num},       {"sample_strategy", p.sample_strategy},
      {"graph_id", p.graph_id}};
}
void from_json(const nlohmann::json& j, RequestAddChannel& p) {
  if (j.count("url") == 0 || j.count("source_type") == 0 ||
      j.count("channel_id") == 0) {
    p.errorCode = ErrorCode::ERR_STREAM_INVALID_VALUE;
    return;
  }

  p.channel_id = j.at("channel_id").get<int>();
  p.url = j.at("url").get<std::string>();
  p.source_type = j.at("source_type").get<std::string>();
  if (j.count("sample_interval")) {
    p.sample_interval = j.at("sample_interval").get<int>();
  }
  if (j.count("decode_id")) {
    p.decode_id = j.at("decode_id").get<int>();
  }
  if (j.count("fps")) {
    p.fps = j.at("fps").get<float>();
  }
  if (j.count("loop_num")) {
    p.loop_num = j.at("loop_num").get<int>();
  }
  if (j.count("sample_strategy"))
    p.sample_strategy = j.at("sample_strategy").get<std::string>();
  if (j.count("graph_id")) {
    p.graph_id = j.at("graph_id").get<int>();
  }
}
bool str_to_object(const std::string& strjson, RequestAddChannel& request) {
  nlohmann::json json_object = nlohmann::json::parse(strjson);
  request = json_object;
  return request.errorCode == ErrorCode::SUCCESS ? true : false;
}

void to_json(nlohmann::json& j, const RequestStopChannel& p) {
  j = nlohmann::json{{"channel_id", p.channel_id},
                     {"decode_id", p.decode_id},
                     {"graph_id", p.graph_id}};
}
void from_json(const nlohmann::json& j, RequestStopChannel& p) {
  if (j.count("channel_id") == 0) {
    p.errorCode = ErrorCode::ERR_STREAM_INVALID_VALUE;
    return;
  }
  p.channel_id = j.at("channel_id").get<int>();

  if (j.count("decode_id")) {
    p.decode_id = j.at("decode_id").get<int>();
  }
  if (j.count("graph_id")) {
    p.graph_id = j.at("graph_id").get<int>();
  }
}
bool str_to_object(const std::string& strjson, RequestStopChannel& request) {
  nlohmann::json json_object = nlohmann::json::parse(strjson);
  request = json_object;
  return request.errorCode == ErrorCode::SUCCESS ? true : false;
}

}  // namespace common
}  // namespace sophon_stream