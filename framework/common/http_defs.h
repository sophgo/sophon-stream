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

}  // namespace common
}  // namespace sophon_stream

#endif