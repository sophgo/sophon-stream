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

#include "common/common_defs.h"
#include "common/logger.h"
#include "engine.h"

constexpr const char* JSON_CONFIG_GRAPH_ID_FILED = "graph_id";
constexpr const char* JSON_CONFIG_DEVICE_ID_FILED = "device_id";
constexpr const char* JSON_CONFIG_ELEMENTS_FILED = "elements";
constexpr const char* JSON_CONFIG_CONNECTION_FILED = "connections";
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
constexpr const char* JSON_CONFIG_INNER_ELEMENTS_ID = "inner_elements_id";

void parse_element_json(
    const nlohmann::detail::iter_impl<nlohmann::json> elements_it,
    nlohmann::json& elementsConfigure, int device_id,
    std::vector<std::pair<int, int>>& src_id_port,
    std::vector<std::pair<int, int>>& sink_id_port) {
  std::ifstream elem_stream;
  for (auto& element_it : *elements_it) {
    nlohmann::json element;
    std::string elem_config =
        element_it.find(JSON_CONFIG_ELEMENT_CONFIG_FILED)->get<std::string>();
    elem_stream.open(elem_config);
    STREAM_CHECK(elem_stream.is_open(), "Please check if element config file ",
                 elem_config, " file exists.");
    elem_stream >> element;
    element["id"] = element_it.find(JSON_CONFIG_ELEMENT_ID_FILED)->get<int>();
    element["device_id"] = device_id;

    std::vector<int> elements_id_list;

    auto inner_it = element_it.find(JSON_CONFIG_INNER_ELEMENTS_ID);
    if (inner_it != element_it.end()) {
      elements_id_list = inner_it->get<std::vector<int>>();
      element[JSON_CONFIG_INNER_ELEMENTS_ID] = elements_id_list;
    }

    auto ports_it = element_it.find(JSON_CONFIG_PORTS_CONFIG_FILED);
    if (ports_it != element_it.end()) {
      auto input_it = ports_it->find(JSON_CONFIG_INPUT_CONFIG_FILED);
      auto output_it = ports_it->find(JSON_CONFIG_OUTPUT_CONFIG_FILED);

      if (input_it != ports_it->end()) {
        for (auto& input : *input_it) {
          if (input.find(JSON_CONFIG_ELEMENT_IS_SRC_FILED)->get<bool>()) {
            src_id_port.push_back(
                {element["id"],
                 input.find(JSON_CONFIG_PORT_ID_FILED)->get<int>()});
          }
        }
      }
      if (output_it != ports_it->end()) {
        for (auto& output : *output_it) {
          if (output.find(JSON_CONFIG_ELEMENT_IS_SINK_FILED)->get<bool>()) {
            sink_id_port.push_back(
                {element["id"],
                 output.find(JSON_CONFIG_PORT_ID_FILED)->get<int>()});
            element["is_sink"] = true;
            if (elements_id_list.size() != 0) {
              // 针对 末尾element是group_element的情况
              sink_id_port.back().first = elements_id_list.back();
            }
          }
        }
      }
    }
    elementsConfigure.push_back(element);
    elem_stream.close();
  }
  STREAM_CHECK((src_id_port.size() > 0 && sink_id_port.size() > 0),
               "THERE MUST BE ONE SRC PORT AND AT LEAST ONE "
               "SINK PORT IN GRAPH! CHECK YOUR ENGINE.JSON.");
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

void init_engine(
    sophon_stream::framework::Engine& engine, nlohmann::json& engine_json,
    const sophon_stream::framework::Engine::SinkHandler& sinkHandler,
    std::map<int, std::vector<std::pair<int, int>>>& graph_src_id_port_map) {
  for (auto& graph_it : engine_json) {
    nlohmann::json graphConfigure, elementsConfigure;
    std::vector<std::pair<int, int>> src_id_port;   // src_port
    std::vector<std::pair<int, int>> sink_id_port;  // sink_port

    int graph_id = graph_it.find(JSON_CONFIG_GRAPH_ID_FILED)->get<int>();
    graphConfigure["graph_id"] = graph_id;
    int device_id = graph_it.find(JSON_CONFIG_DEVICE_ID_FILED)->get<int>();
    auto elements_it = graph_it.find(JSON_CONFIG_ELEMENTS_FILED);
    parse_element_json(elements_it, elementsConfigure, device_id, src_id_port,
                       sink_id_port);
    graphConfigure["elements"] = elementsConfigure;
    auto connect_it = graph_it.find(JSON_CONFIG_CONNECTION_FILED);
    parse_connection_json(connect_it, graphConfigure);

    engine.addGraph(graphConfigure.dump());
    for (auto& sink_id_port_obj : sink_id_port) {
      engine.setSinkHandler(graph_id, sink_id_port_obj.first,
                            sink_id_port_obj.second, sinkHandler);
    }
    graph_src_id_port_map[graph_id] = src_id_port;
  }
}
