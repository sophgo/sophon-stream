#include <sys/stat.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "common/Clocker.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/logger.h"
#include "decode.h"
#include "engine.h"
#include "gtest/gtest.h"
#include "init_engine.h"

typedef struct usecase_config_ {
  int num_graphs;
  int num_channels_per_graph;
  std::vector<nlohmann::json> channel_configs;
  std::string engine_config_file;
} usecase_config;

constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FILED = "channels";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_CHANNEL_ID_FILED = "channel_id";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_URL_FILED = "url";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SOURCE_TYPE_FILED = "source_type";

usecase_config parse_usecase_json(std::string& json_path) {
  std::ifstream istream;
  istream.open(json_path);
  assert(istream.is_open());
  nlohmann::json usecase_json;
  istream >> usecase_json;
  istream.close();

  usecase_config config;

  auto channel_config_it = usecase_json.find(JSON_CONFIG_CHANNEL_CONFIG_FILED);
  for (auto& channel_it : *channel_config_it) {
    nlohmann::json channel_json;
    channel_json["channel_id"] = 
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_CHANNEL_ID_FILED)
          ->get<int>();
    channel_json["url"] = 
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_URL_FILED)
          ->get<std::string>();
    channel_json["source_type"] = 
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_SOURCE_TYPE_FILED)
          ->get<int>();
    config.channel_configs.push_back(channel_json);
  }

  config.engine_config_file =
      usecase_json.find(JSON_CONFIG_ENGINE_CONFIG_PATH_FILED)
          ->get<std::string>();

  return config;
}

TEST(TestYoloxBytetrackOsdEncode, TestYoloxBytetrackOsdEncode) {
  ::logInit("debug", "");

  std::mutex mtx;
  std::condition_variable cv;

  sophon_stream::Clocker clocker;
  std::atomic_uint32_t frameCount(0);
  std::atomic_int32_t finishedChannelCount(0);

  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();

  std::ifstream istream;
  nlohmann::json engine_json;
  std::string config_file = "../config/usecase.json";
  usecase_config usecase_json = parse_usecase_json(config_file);

  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(usecase_json.engine_config_file);
  assert(istream.is_open());
  istream >> engine_json;
  istream.close();

  usecase_json.num_graphs = engine_json.size();
  usecase_json.num_channels_per_graph = usecase_json.channel_configs.size();
  int num_channels =
      usecase_json.num_channels_per_graph * usecase_json.num_graphs;

  auto stopHandler = [&](std::shared_ptr<void> data) {
    // write stop data handler here
    auto objectMetadata =
        std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
    if (objectMetadata == nullptr) return;
    frameCount++;
    if (objectMetadata->mFrame->mEndOfStream) {
      printf("meet a eof\n");
      finishedChannelCount++;
      if (finishedChannelCount == num_channels) {
        cv.notify_one();
      }
      return;
    }
  };

  std::map<int, std::pair<int, int>> graph_src_id_port_map;
  init_engine(engine, engine_json, stopHandler, graph_src_id_port_map);

  for (auto graph_id : engine.getGraphIds()) {
    for (auto& channel_config : usecase_json.channel_configs) {
      auto channelTask =
          std::make_shared<sophon_stream::element::decode::ChannelTask>();
      channelTask->request.operation = sophon_stream::element::decode::
          ChannelOperateRequest::ChannelOperate::START;
      channelTask->request.channelId = channel_config["channel_id"];
      channelTask->request.json = channel_config.dump();
      std::pair<int, int> src_id_port = graph_src_id_port_map[graph_id];
      sophon_stream::common::ErrorCode errorCode =
          engine.pushInputData(graph_id, src_id_port.first, src_id_port.second,
                               std::static_pointer_cast<void>(channelTask));
    }
  }

  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for (int i = 0; i < usecase_json.num_graphs; i++) {
    std::cout << "graph stop" << std::endl;
    engine.stop(i);
  }
  long totalCost = clocker.tell_us();
  std::cout << " total time cost " << totalCost << " us." << std::endl;
  double fps = static_cast<double>(frameCount) / totalCost;
  std::cout << "frame count is " << frameCount << " | fps is " << fps * 1000000
            << " fps." << std::endl;
}
