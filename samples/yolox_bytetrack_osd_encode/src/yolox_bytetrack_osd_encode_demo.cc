#include <sys/stat.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "common/clocker.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "decode.h"
#include "engine.h"
#include "init_engine.h"

typedef struct demo_config_ {
  int num_graphs;
  int num_channels_per_graph;
  std::vector<nlohmann::json> channel_configs;
  std::string engine_config_file;
} demo_config;

constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FILED = "channels";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_CHANNEL_ID_FILED =
    "channel_id";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_URL_FILED = "url";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SOURCE_TYPE_FILED =
    "source_type";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_LOOP_NUM_FILED = "loop_num";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FPS_FILED = "fps";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SAMPLE_INTERVAL_FILED = "sample_interval";

demo_config parse_demo_json(std::string& json_path) {
  std::ifstream istream;
  istream.open(json_path);
  assert(istream.is_open());
  nlohmann::json demo_json;
  istream >> demo_json;
  istream.close();

  demo_config config;

  auto channel_config_it = demo_json.find(JSON_CONFIG_CHANNEL_CONFIG_FILED);
  for (auto& channel_it : *channel_config_it) {
    nlohmann::json channel_json;
    channel_json["channel_id"] =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_CHANNEL_ID_FILED)
            ->get<int>();
    channel_json["url"] = channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_URL_FILED)
                              ->get<std::string>();
    channel_json["source_type"] =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_SOURCE_TYPE_FILED)
            ->get<std::string>();

    channel_json["loop_num"] = 1;
    auto loop_num_it =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_LOOP_NUM_FILED);
    if (channel_it.end() != loop_num_it)
      channel_json["loop_num"] = loop_num_it->get<int>();

    auto fps_it =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_FPS_FILED);
    if (channel_it.end() != fps_it)
      channel_json["fps"] = fps_it->get<double>();

    auto sample_interval_it =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_SAMPLE_INTERVAL_FILED);
    if (channel_it.end() != sample_interval_it)
      channel_json["sample_interval"] = sample_interval_it->get<int>();

    config.channel_configs.push_back(channel_json);
  }

  config.engine_config_file =
      demo_json.find(JSON_CONFIG_ENGINE_CONFIG_PATH_FILED)->get<std::string>();

  return config;
}

int main() {
  ::logInit("debug", "");

  std::mutex mtx;
  std::condition_variable cv;

  sophon_stream::common::Clocker clocker;
  std::atomic_uint32_t frameCount(0);
  std::atomic_int32_t finishedChannelCount(0);

  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();

  std::ifstream istream;
  nlohmann::json engine_json;
  std::string config_file = "../config/yolox_bytetrack_osd_encode_demo.json";
  demo_config demo_json = parse_demo_json(config_file);

  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(demo_json.engine_config_file);
  assert(istream.is_open());
  istream >> engine_json;
  istream.close();

  demo_json.num_graphs = engine_json.size();
  demo_json.num_channels_per_graph = demo_json.channel_configs.size();
  int num_channels = demo_json.num_channels_per_graph * demo_json.num_graphs;

  auto sinkHandler = [&](std::shared_ptr<void> data) {
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
  init_engine(engine, engine_json, sinkHandler, graph_src_id_port_map);

  for (auto graph_id : engine.getGraphIds()) {
    for (auto& channel_config : demo_json.channel_configs) {
      auto channelTask =
          std::make_shared<sophon_stream::element::decode::ChannelTask>();
      channelTask->request.operation = sophon_stream::element::decode::
          ChannelOperateRequest::ChannelOperate::START;
      channelTask->request.channelId = channel_config["channel_id"];
      channelTask->request.json = channel_config.dump();
      std::pair<int, int> src_id_port = graph_src_id_port_map[graph_id];
      sophon_stream::common::ErrorCode errorCode =
          engine.pushSourceData(graph_id, src_id_port.first, src_id_port.second,
                                std::static_pointer_cast<void>(channelTask));
    }
  }

  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for (int i = 0; i < demo_json.num_graphs; i++) {
    std::cout << "graph stop" << std::endl;
    engine.stop(i);
  }
  long totalCost = clocker.tell_us();
  std::cout << " total time cost " << totalCost << " us." << std::endl;
  double fps = static_cast<double>(frameCount) / totalCost;
  std::cout << "frame count is " << frameCount << " | fps is " << fps * 1000000
            << " fps." << std::endl;
  return 0;
}
