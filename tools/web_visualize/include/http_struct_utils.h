#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

struct PipelineListResponseData {
  int id;
  std::string name;
  std::string size;
  std::string created_at;
};

struct PipelineListResponse {
  std::string status;
  std::string message;
  std::vector<PipelineListResponseData> data;
};

struct RunRequest {
  int pipeline_id;
};

struct RunResponseData {
  int pipeline_id;
  std::string status;
};

struct RunResponse {
  std::string status;
  std::string message;
  RunResponseData data;
};

void to_json(nlohmann::json& j, const PipelineListResponseData& p);
void to_json(nlohmann::json& j, const PipelineListResponse& p);

void from_json(const nlohmann::json& j, RunRequest& p);
void to_json(nlohmann::json& j, const RunResponseData& p);
void to_json(nlohmann::json& j, const RunResponse& p);