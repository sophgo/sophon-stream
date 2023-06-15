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
#include "engine.h"

constexpr const char* JSON_CONFIG_GRAPH_ID_FILED = "graph_id";
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

void parse_element_json(
    const nlohmann::detail::iter_impl<nlohmann::json> elements_it,
    nlohmann::json& elementsConfigure, std::pair<int, int>& src_id_port,
    std::pair<int, int>& sink_id_port) {

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

void init_engine(
    sophon_stream::framework::Engine& engine, nlohmann::json& engine_json,
    const sophon_stream::framework::Engine::SinkHandler& sinkHandler,
    std::map<int, std::pair<int, int>>& graph_src_id_port_map) {
  for (auto& graph_it : engine_json) {
    nlohmann::json graphConfigure, elementsConfigure;
    std::pair<int, int> src_id_port = {-1, -1};   // src_port
    std::pair<int, int> sink_id_port = {-1, -1};  // sink_port

    int graph_id = graph_it.find(JSON_CONFIG_GRAPH_ID_FILED)->get<int>();
    graphConfigure["graph_id"] = graph_id;
    auto elements_it = graph_it.find(JSON_CONFIG_ELEMENTS_FILED);
    parse_element_json(elements_it, elementsConfigure, src_id_port,
                       sink_id_port);
    graphConfigure["elements"] = elementsConfigure;
    auto connect_it = graph_it.find(JSON_CONFIG_CONNECTION_FILED);
    parse_connection_json(connect_it, graphConfigure);

    engine.addGraph(graphConfigure.dump());
    engine.setSinkHandler(graph_id, sink_id_port.first, sink_id_port.second,
                          sinkHandler);
    graph_src_id_port_map[graph_id] = src_id_port;
  }
}