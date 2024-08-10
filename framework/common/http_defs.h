//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_HTTP_DEFS_H_
#define SOPHON_STREAM_COMMON_HTTP_DEFS_H_

#include "nlohmann/json.hpp"
#include "error_code.h"

namespace sophon_stream {
namespace common {

struct Result {
  int status;
  std::string taskId;
};
void from_json(const nlohmann::json& j, Result& p);
void to_json(nlohmann::json& j, const std::vector<Result>& p);

struct Response {
  int code;
  std::string msg;
  std::vector<Result> results;
};
void to_json(nlohmann::json& j, const Response& p);
void from_json(const nlohmann::json& j, Response& p);

struct RequestSingleInt {
  int idx;
};
void to_json(nlohmann::json& j, const RequestSingleInt& p);
void from_json(const nlohmann::json& j, RequestSingleInt& p);
bool str_to_object(const std::string& strjson, RequestSingleInt& request);

struct RequestSingleFloat {
  float value;
};
void to_json(nlohmann::json& j, const RequestSingleFloat& p);
void from_json(const nlohmann::json& j, RequestSingleFloat& p);
bool str_to_object(const std::string& strjson, RequestSingleFloat& request);

struct RequestAddChannel {
  int channel_id;
  std::string url;
  std::string source_type;
  int sample_interval = 1;
  int decode_id = -1;
  float fps = 1;
  int loop_num = 1;
  std::string sample_strategy = "DROP";
  int graph_id = 0;
  ErrorCode errorCode = ErrorCode::SUCCESS;
};
void to_json(nlohmann::json& j, const RequestAddChannel& p);
void from_json(const nlohmann::json& j, RequestAddChannel& p);
bool str_to_object(const std::string& strjson, RequestAddChannel& request);

struct RequestStopChannel {
  int channel_id;
  int decode_id = -1;
  int graph_id = 0;
  ErrorCode errorCode = ErrorCode::SUCCESS;
};
void to_json(nlohmann::json& j, const RequestStopChannel& p);
void from_json(const nlohmann::json& j, RequestStopChannel& p);
bool str_to_object(const std::string& strjson, RequestStopChannel& request);


}  // namespace common
}  // namespace sophon_stream

#endif