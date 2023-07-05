
#include "http_struct_utils.h"

void to_json(nlohmann::json& j, const PipelineListResponseData& p) {
  j = nlohmann::json{{"id", p.id},
                     {"name", p.name},
                     {"size", p.size},
                     {"created_at", p.created_at}};
}

void to_json(nlohmann::json& j, const PipelineListResponse& p) {
  j = nlohmann::json{
      {"status", p.status}, {"message", p.message}, {"data", p.data}};
}

void from_json(const nlohmann::json& j, RunRequest& p) {
  p.pipeline_id = j.at("pipeline_id").get<int>();
}

void to_json(nlohmann::json& j, const RunResponseData& p) {
  j = nlohmann::json{{"status", p.status}, {"pipeline_id", p.pipeline_id}};
}

void to_json(nlohmann::json& j, const RunResponse& p) {
  j = nlohmann::json{
      {"status", p.status}, {"message", p.message}, {"data", p.data}};
}