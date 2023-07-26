//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef HTTP_STRUCT_UTILS_H_
#define HTTP_STRUCT_UTILS_H_
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

struct PipelineResponseData {
  int pipeline_id;
  std::string pipeline_name;
  bool is_running;
  std::vector<std::map<std::string, std::string>> json_list;
};

struct PipelineResponse {
  std::string status;
  std::string message;
  std::vector<PipelineResponseData> data;
};

struct PipelineRequest {
  bool is_running;
};

struct JsonResponseData {
  int json_id;
  std::string json_name;
  nlohmann::json json_content;
};

struct JsonResponse {
  std::string status;
  std::string message;
  JsonResponseData data;
};

struct ErrorResponse {
  int error_code;
  std::string status;
  std::string message;
};

struct JsonsList {
  std::vector<std::map<std::string, std::string>> jsons_list;
};

void to_json(nlohmann::json& j, const PipelineResponseData& p);
void to_json(nlohmann::json& j, const PipelineResponse& p);

void to_json(nlohmann::json& j, const JsonResponseData& p);
void to_json(nlohmann::json& j, const JsonResponse& p);

void to_json(nlohmann::json& j, const ErrorResponse& p);

void from_json(const nlohmann::json& j, PipelineRequest& p);

PipelineResponseData read_json_preview(const std::string& pipeline_id);
JsonsList read_jsons_list();

#endif //HTTP_STRUCT_UTILS_H_