//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include <fstream>
#include <nlohmann/json.hpp>

#include "common/logger.h"

typedef struct usecase_config_ {
  int num_graphs;
  int num_channels_per_graph;
  std::vector<nlohmann::json> decodeConfigures;
  bool download_image;
  std::string engine_config_file;
  std::vector<std::string> class_names;
} usecase_config;

constexpr const char* JSON_CONFIG_NUM_GRAPHS_FILED = "num_graphs";
constexpr const char* JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED =
    "num_channels_per_graph";
constexpr const char* JSON_CONFIG_URLS_FILED = "urls";
constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";
constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CLASS_NAMES_FILED = "class_names";
constexpr const char* JSON_CONFIG_DECODE_CONFIGURE_FILED = "decodeConfigure";

usecase_config parse_usecase_json(std::string& json_path) {
  std::ifstream istream;
  istream.open(json_path);
  assert(istream.is_open());
  nlohmann::json usecase_json;
  istream >> usecase_json;
  istream.close();

  usecase_config config;

  config.num_graphs =
      usecase_json.find(JSON_CONFIG_NUM_GRAPHS_FILED)->get<int>();
  config.num_channels_per_graph =
      usecase_json.find(JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED)->get<int>();

  auto decodeConfigures_it =
      usecase_json.find(JSON_CONFIG_DECODE_CONFIGURE_FILED);
  assert(decodeConfigures_it->size() == config.num_graphs);
  for (auto& decodeConfigure_it : *decodeConfigures_it)
    config.decodeConfigures.push_back(decodeConfigure_it);

  config.download_image =
      usecase_json.find(JSON_CONFIG_DOWNLOAD_IMAGE_FILED)->get<bool>();
  config.engine_config_file =
      usecase_json.find(JSON_CONFIG_ENGINE_CONFIG_PATH_FILED)
          ->get<std::string>();
  std::string class_names_file =
      usecase_json.find(JSON_CONFIG_CLASS_NAMES_FILED)->get<std::string>();

  istream.open(class_names_file);
  if (istream.is_open()) {
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length() - 1);
      config.class_names.push_back(line);
    }
  }
  istream.close();
  if (config.download_image) {
    const char* dir_path = "./results";
    struct stat info;
    if (stat(dir_path, &info) == 0 && S_ISDIR(info.st_mode)) {
      std::cout << "Directory already exists." << std::endl;
    } else {
      if (mkdir(dir_path, 0777) == 0) {
        std::cout << "Directory created successfully." << std::endl;
      } else {
        std::cerr << "Error creating directory." << std::endl;
      }
    }
  }

  return std::move(config);
}

constexpr const char* JSON_CONFIG_ELEMENT_CONFIG_FILED = "element_config";
constexpr const char* JSON_CONFIG_ELEMENT_ID_FILED = "element_id";
constexpr const char* JSON_CONFIG_PORTS_CONFIG_FILED = "ports";
constexpr const char* JSON_CONFIG_INPUT_CONFIG_FILED = "input";
constexpr const char* JSON_CONFIG_OUTPUT_CONFIG_FILED = "output";
constexpr const char* JSON_CONFIG_ELEMENT_IS_SINK_FILED = "is_sink";
constexpr const char* JSON_CONFIG_ELEMENT_IS_SRC_FILED = "is_src";
constexpr const char* JSON_CONFIG_PORT_ID_FILED = "port_id";
constexpr const char* JSON_CONFIG_SRC_ID_FILED = "src_element_id";
constexpr const char* JSON_CONFIG_SRC_PORT_FILED = "src_port";
constexpr const char* JSON_CONFIG_DST_ID_FILED = "dst_element_id";
constexpr const char* JSON_CONFIG_DST_PORT_FILED = "dst_port";

void parse_element_json(
    const nlohmann::detail::iter_impl<nlohmann::json> elements_it,
    nlohmann::json& elementsConfigure, std::pair<int, int>& src_id_port,
    std::pair<int, int>& sink_id_port) {
  ::logInit("debug", "");

  std::ifstream elem_stream;
  for (auto& element_it : *elements_it) {
    nlohmann::json element;
    std::string elem_config =
        element_it.find(JSON_CONFIG_ELEMENT_CONFIG_FILED)->get<std::string>();
    elem_stream.open(elem_config);
    assert(elem_stream.is_open());
    elem_stream >> element;
    element["id"] = element_it.find(JSON_CONFIG_ELEMENT_ID_FILED)->get<int>();

    auto ports_it = element_it.find(JSON_CONFIG_PORTS_CONFIG_FILED);
    auto input_it = ports_it->find(JSON_CONFIG_INPUT_CONFIG_FILED);
    auto output_it = ports_it->find(JSON_CONFIG_OUTPUT_CONFIG_FILED);

    for (auto& input : *input_it) {
      if (input.find(JSON_CONFIG_ELEMENT_IS_SRC_FILED)->get<bool>()) {
        if (src_id_port.first != -1) {
          IVS_ERROR("Too many src element");
          abort();
        }
        src_id_port = {element["id"],
                       input.find(JSON_CONFIG_PORT_ID_FILED)->get<int>()};
      }
    }
    for (auto& output : *output_it) {
      if (output.find(JSON_CONFIG_ELEMENT_IS_SINK_FILED)->get<bool>()) {
        if (sink_id_port.first != -1) {
          IVS_ERROR("Too many sink element");
          abort();
        }
        sink_id_port = {element["id"],
                        output.find(JSON_CONFIG_PORT_ID_FILED)->get<int>()};
        element["is_sink"] = true;
      }
    }

    elementsConfigure.push_back(element);
    elem_stream.close();
  }
}

void parse_connection_json(
    nlohmann::detail::iter_impl<nlohmann::json>& connect_it,
    nlohmann::json& graphConfigure) {
  for (auto connect_config : *connect_it) {
    int src_element_id =
        connect_config.find(JSON_CONFIG_SRC_ID_FILED)->get<int>();
    int src_port = connect_config.find(JSON_CONFIG_SRC_PORT_FILED)->get<int>();
    int dst_element_id =
        connect_config.find(JSON_CONFIG_DST_ID_FILED)->get<int>();
    int dst_port = connect_config.find(JSON_CONFIG_DST_PORT_FILED)->get<int>();
    nlohmann::json connectConf;
    connectConf["src_id"] = src_element_id;
    connectConf["src_port"] = src_port;
    connectConf["dst_id"] = dst_element_id;
    connectConf["dst_port"] = dst_port;
    graphConfigure["connections"].push_back(connectConf);
  }
}