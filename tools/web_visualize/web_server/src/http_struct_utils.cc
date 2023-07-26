//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "http_struct_utils.h"

#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

void to_json(nlohmann::json& j, const PipelineResponseData& p) {
  j = nlohmann::json{{"pipeline_id", p.pipeline_id},
                     {"pipeline_name", p.pipeline_name},
                     {"is_running", p.is_running},
                     {"json_list", p.json_list}};
}

void to_json(nlohmann::json& j, const PipelineResponse& p) {
  j = nlohmann::json{
      {"status", p.status}, {"message", p.message}, {"data", p.data}};
}

void to_json(nlohmann::json& j, const JsonResponseData& p) {
  j = nlohmann::json{{"json_id", p.json_id},
                     {"json_name", p.json_name},
                     {"json_content", p.json_content}};
}

void to_json(nlohmann::json& j, const JsonResponse& p) {
  j = nlohmann::json{
      {"status", p.status}, {"message", p.message}, {"data", p.data}};
}

void to_json(nlohmann::json& j, const ErrorResponse& p) {
  j = nlohmann::json{{"status", p.status},
                     {"message", p.message},
                     {"error_code", p.error_code}};
}

void from_json(const nlohmann::json& j, PipelineRequest& p) {
  p.is_running = j.at("is_running").get<bool>();
}

PipelineResponseData read_json_preview(const std::string& str_pipeline_id) {
  std::ifstream file("../config/all_files_preview.json");
  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  nlohmann::json js = nlohmann::json::parse(buffer.str());
  PipelineResponse response;
  for (const auto& item : js) {
    PipelineResponseData pipelineData;
    pipelineData.pipeline_id = item["pipeline_id"];
    pipelineData.pipeline_name = item["pipeline_name"];
    pipelineData.is_running = item["is_running"];
    for (const auto& jsonItem : item["json_list"]) {
      std::map<std::string, std::string> jsonMap;
      jsonMap["json_id"] = jsonItem["json_id"];
      jsonMap["json_name"] = jsonItem["json_name"];
      pipelineData.json_list.push_back(jsonMap);
    }
    response.data.push_back(pipelineData);
  }
  return response.data[std::stoi(str_pipeline_id)];
}

JsonsList read_jsons_list() {
  JsonsList jsonslist;
  std::ifstream file("../config/all_files_preview.json");
  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  nlohmann::json js = nlohmann::json::parse(buffer.str());
  PipelineResponse response;
  for (const auto& item : js) {
    for (const auto& jsonItem : item["json_list"]) {
      std::map<std::string, std::string> jsonMap;
      jsonMap[jsonItem["json_id"]] = jsonItem["json_name"];
      jsonslist.jsons_list.push_back(jsonMap);
    }
  }
  return jsonslist;
}
