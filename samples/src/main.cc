//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//
#include "draw_funcs.h"
#include <functional>

typedef struct demo_config_ {
  int num_graphs;
  int num_channels_per_graph;
  std::vector<nlohmann::json> channel_configs;
  bool download_image;
  std::string engine_config_file;
  std::vector<std::string> class_names;
  std::string draw_func_name;
  std::vector<std::string> car_attr; 
  std::vector<std::string> person_attr;
  std::string heatmap_loss;
} demo_config;

constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";
constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CLASS_NAMES_FILED = "class_names";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FILED = "channels";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_CHANNEL_ID_FILED =
    "channel_id";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_URL_FILED = "url";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SOURCE_TYPE_FILED =
    "source_type";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_LOOP_NUM_FILED = "loop_num";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FPS_FILED = "fps";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SAMPLE_INTERVAL_FILED =
    "sample_interval";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SKIP_ELEMENT_FILED =
    "skip_element";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_SAMPLE_STRATEGY_FILED =
    "sample_strategy";
constexpr const char* JSON_CONFIG_DRAW_FUNC_NAME_FILED = "draw_func_name";
constexpr const char* JSON_CONFIG_CAR_ATTRIBUTES_FILED = "car_attributes";
constexpr const char* JSON_CONFIG_PERSON_ATTRIBUTES_FILED = "person_attributes";
constexpr const char* JSON_CONFIG_CHANNEL_DECODE_IDX_FILED = "decode_idx";
constexpr const char* JSON_CONFIG_HEATMAP_LOSS_CONFIG_FILED = "heatmap_loss";

demo_config parse_demo_json(std::string& json_path) {
  std::ifstream istream;
  istream.open(json_path);
  STREAM_CHECK(istream.is_open(), "Please check config file ", json_path,
               " exists.");
  nlohmann::json demo_json;
  istream >> demo_json;
  istream.close();

  demo_config config;
  auto channel_config_it = demo_json.find(JSON_CONFIG_CHANNEL_CONFIG_FILED);

  config.download_image = false;
  if (demo_json.contains(JSON_CONFIG_DOWNLOAD_IMAGE_FILED))
      config.download_image = demo_json.find(JSON_CONFIG_DOWNLOAD_IMAGE_FILED)->get<bool>();
  config.engine_config_file =
      demo_json.find(JSON_CONFIG_ENGINE_CONFIG_PATH_FILED)->get<std::string>();
  std::string class_names_file;
  if (demo_json.contains(JSON_CONFIG_CLASS_NAMES_FILED))
    class_names_file =
        demo_json.find(JSON_CONFIG_CLASS_NAMES_FILED)->get<std::string>();
  if (demo_json.contains(JSON_CONFIG_DRAW_FUNC_NAME_FILED))
      config.draw_func_name =
        demo_json.find(JSON_CONFIG_DRAW_FUNC_NAME_FILED)->get<std::string>();
  else
    config.draw_func_name = "default";
  std::string car_attr_file;
  if (demo_json.contains(JSON_CONFIG_CAR_ATTRIBUTES_FILED))
    car_attr_file =
        demo_json.find(JSON_CONFIG_CAR_ATTRIBUTES_FILED)->get<std::string>();
  std::string person_attr_file;
  if (demo_json.contains(JSON_CONFIG_PERSON_ATTRIBUTES_FILED))
    person_attr_file =
        demo_json.find(JSON_CONFIG_PERSON_ATTRIBUTES_FILED)->get<std::string>();
  if (demo_json.contains(JSON_CONFIG_HEATMAP_LOSS_CONFIG_FILED))
    config.heatmap_loss =
      demo_json.find(JSON_CONFIG_HEATMAP_LOSS_CONFIG_FILED)->get<std::string>();

  if (config.download_image) {
    const char* dir_path = "./results";
    struct stat info;
    if (stat(dir_path, &info) == 0 && S_ISDIR(info.st_mode)) {
      std::cout << "Directory already exists." << std::endl;
      int new_permissions = S_IRWXU | S_IRWXG | S_IRWXO;
      if (chmod(dir_path, new_permissions) == 0) {
        std::cout << "Directory permissions modified successfully."
                  << std::endl;
      } else {
        std::cerr << "Error modifying directory permissions." << std::endl;
        abort();
      }
    } else {
      if (mkdir(dir_path, 0777) == 0) {
        std::cout << "Directory created successfully." << std::endl;
      } else {
        std::cerr << "Error creating directory." << std::endl;
        abort();
      }
    }

    std::string line;
    if (demo_json.contains(JSON_CONFIG_CLASS_NAMES_FILED)) {
      istream.open(class_names_file);
      assert(istream.is_open());
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        config.class_names.push_back(line);
      }
      istream.close();
    }

    if (demo_json.contains(JSON_CONFIG_CAR_ATTRIBUTES_FILED)) {
      istream.open(car_attr_file);
      assert(istream.is_open());
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        config.car_attr.push_back(line);
      }
      istream.close();
    }
    if (demo_json.contains(JSON_CONFIG_PERSON_ATTRIBUTES_FILED)) {
      istream.open(person_attr_file);
      assert(istream.is_open());
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        config.person_attr.push_back(line);
      }
      istream.close();
    }
  }

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

    auto fps_it = channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_FPS_FILED);
    if (channel_it.end() != fps_it) channel_json["fps"] = fps_it->get<double>();

    auto sample_interval_it =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_SAMPLE_INTERVAL_FILED);
    if (channel_it.end() != sample_interval_it)
      channel_json["sample_interval"] = sample_interval_it->get<int>();

    auto sample_strategy_it =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_SAMPLE_STRATEGY_FILED);
    if (channel_it.end() != sample_strategy_it)
      channel_json["sample_strategy"] = sample_strategy_it->get<std::string>();

    auto skip_element_it =
        channel_it.find(JSON_CONFIG_CHANNEL_CONFIG_SKIP_ELEMENT_FILED);
    if (skip_element_it != channel_it.end()) {
      channel_json["skip_element"] = skip_element_it->get<std::vector<int>>();
    }

    channel_json["decode_idx"] = 0;
    auto decode_idx_it = channel_it.find(JSON_CONFIG_CHANNEL_DECODE_IDX_FILED);
    if (decode_idx_it != channel_it.end()) {
      channel_json["decode_idx"] = decode_idx_it->get<int>();
    }

    config.channel_configs.push_back(channel_json);
  }

  return config;
}

int main(int argc, char *argv[]) {
  const char *keys="{demo_config_path | ../license_plate_recognition/config/license_plate_recognition_demo.json | demo config path}"
                    "{help | 0 | print help information.}";
  cv::CommandLineParser parser(argc, argv, keys);
  if (parser.get<bool>("help")) {
    parser.printMessage();
    return 0;
  }
  std::string demo_config_fpath = parser.get<std::string>("demo_config_path");

  ::logInit("debug", "");

  std::mutex mtx;
  std::condition_variable cv;

  sophon_stream::common::Clocker clocker;
  std::atomic_uint32_t frameCount(0);
  std::atomic_int32_t finishedChannelCount(0);

  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();

  std::ifstream istream;
  nlohmann::json engine_json;
  demo_config demo_json = parse_demo_json(demo_config_fpath);

  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(demo_json.engine_config_file);
  STREAM_CHECK(istream.is_open(), "Please check if engine_config_file ",
               demo_json.engine_config_file, " exists.");
  istream >> engine_json;
  istream.close();

  demo_json.num_graphs = engine_json.size();
  demo_json.num_channels_per_graph = demo_json.channel_configs.size();
  int num_channels =
      demo_json.num_channels_per_graph * demo_json.num_graphs;
  std::vector<::sophon_stream::common::FpsProfiler> fpsProfilers(num_channels);
  for (int i = 0; i < num_channels; ++i) {
    std::string fpsName = "channel_" + std::to_string(i);
    fpsProfilers[i].config(fpsName, 100);
  }

  std::function<void(std::shared_ptr<sophon_stream::common::ObjectMetadata>)> draw_func;
  std::string out_dir = "./results";
  if (demo_json.draw_func_name == "draw_bytetrack_results") draw_func = std::bind(draw_bytetrack_results, std::placeholders::_1, out_dir);
  else if (demo_json.draw_func_name == "draw_license_plate_recognition_results") draw_func = std::bind(draw_license_plate_recognition_results, std::placeholders::_1, out_dir);
  else if (demo_json.draw_func_name == "draw_openpose_results") draw_func = std::bind(draw_openpose_results, std::placeholders::_1, out_dir);
  else if (demo_json.draw_func_name == "draw_retinaface_results") draw_func = std::bind(draw_retinaface_results, std::placeholders::_1, out_dir);
  else if (demo_json.draw_func_name == "draw_retinaface_distributor_resnet_faiss_converger_results") draw_func = std::bind(draw_retinaface_distributor_resnet_faiss_converger_results, std::placeholders::_1, out_dir);
  else if (demo_json.draw_func_name == "draw_yolov5_results") draw_func = std::bind(draw_yolov5_results, std::placeholders::_1, out_dir, demo_json.class_names);
  else if (demo_json.draw_func_name == "draw_yolov5_bytetrack_distributor_resnet_converger_results") draw_func = std::bind(draw_yolov5_bytetrack_distributor_resnet_converger_results, std::placeholders::_1, out_dir, demo_json.car_attr, demo_json.person_attr);
  else if (demo_json.draw_func_name == "draw_yolox_results") draw_func = std::bind(draw_yolox_results, std::placeholders::_1, out_dir, demo_json.class_names);
  else if (demo_json.draw_func_name == "save_only") draw_func = std::bind(save_only, std::placeholders::_1, out_dir);
  else if (demo_json.draw_func_name == "draw_yolov5_fastpose_posec3d_results") draw_func = std::bind(draw_yolov5_fastpose_posec3d_results, std::placeholders::_1, out_dir, demo_json.heatmap_loss);
  else if (demo_json.draw_func_name == "default") draw_func = std::function<void(std::shared_ptr<sophon_stream::common::ObjectMetadata>)>(draw_default);
  else IVS_ERROR("No such function! Please check your 'draw_func_name'.");

  auto sinkHandler = [&, draw_func](std::shared_ptr<void> data) {
    // write stop data handler here
    auto objectMetadata =
        std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
    if (objectMetadata == nullptr) return;
    if (!objectMetadata->mFilter) {
      frameCount++;
      fpsProfilers[objectMetadata->mFrame->mChannelIdInternal].add(1);
    }
    if (objectMetadata->mFrame->mEndOfStream) {
      printf("meet a eof\n");
      finishedChannelCount++;
      if (finishedChannelCount == num_channels) {
        cv.notify_one();
      }
      return;
    }
    if (demo_json.download_image)
        draw_func(objectMetadata);
  };

  std::map<int, std::vector<std::pair<int, int>>> graph_src_id_port_map;
  init_engine(engine, engine_json, sinkHandler, graph_src_id_port_map);

  for (auto graph_id : engine.getGraphIds()) {
    for (auto& channel_config : demo_json.channel_configs) {
      auto channelTask =
          std::make_shared<sophon_stream::element::decode::ChannelTask>();
      channelTask->request.operation = sophon_stream::element::decode::
          ChannelOperateRequest::ChannelOperate::START;
        channelTask->request.channelId = channel_config["channel_id"];
      channelTask->request.json = channel_config.dump();
      int decode_idx = channel_config["decode_idx"];
      std::pair<int, int> src_id_port = graph_src_id_port_map[graph_id][decode_idx];
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