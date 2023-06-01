#include <sys/stat.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "DecoderElement.h"
#include "common/Clocker.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/logger.h"
#include "config.h"
#include "engine.h"
#include "gtest/gtest.h"

const std::vector<std::vector<int>> colors = {
    {255, 0, 0},    {255, 85, 0},    {255, 170, 0},   {255, 255, 0},
    {170, 255, 0},  {85, 255, 0},    {0, 255, 0},     {0, 255, 85},
    {0, 255, 170},  {0, 255, 255},   {0, 170, 255},   {0, 85, 255},
    {0, 0, 255},    {85, 0, 255},    {170, 0, 255},   {255, 0, 255},
    {255, 0, 170},  {255, 0, 85},    {255, 0, 0},     {255, 0, 255},
    {255, 85, 255}, {255, 170, 255}, {255, 255, 255}, {170, 255, 255},
    {85, 255, 255}};

void draw_bmcv(bm_handle_t& handle, int classId,
               std::vector<std::string>& class_names, float conf, int left,
               int top, int width, int height, bm_image& frame,
               bool put_text_flag)  // Draw the predicted bounding box
{
  int colors_num = colors.size();
  // Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << ","
            << rect.crop_h << std::endl;
  bmcv_image_draw_rectangle(
      handle, frame, 1, &rect, 3, colors[classId % colors_num][0],
      colors[classId % colors_num][1], colors[classId % colors_num][2]);
  if (put_text_flag) {
    std::string label = class_names[classId] + ":" + cv::format("%.2f", conf);
    bmcv_point_t org = {left, top};
    bmcv_color_t color = {colors[classId % colors_num][0],
                          colors[classId % colors_num][1],
                          colors[classId % colors_num][2]};
    int thickness = 2;
    float fontScale = 2;
    if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                          color, fontScale, thickness)) {
      std::cout << "bmcv put text error !!!" << std::endl;
    }
  }
}

constexpr const char* JSON_CONFIG_GRAPH_ID_FILED = "graph_id";
constexpr const char* JSON_CONFIG_NUM_GRAPHS_FILED = "num_graphs";
constexpr const char* JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED =
    "num_channels_per_graph";
constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";
constexpr const char* JSON_CONFIG_GRAPH_NAME_FILED = "graph_name";
constexpr const char* JSON_CONFIG_ELEMENTS_FILED = "elements";
constexpr const char* JSON_CONFIG_ELEMENT_ID_FILED = "element_id";
constexpr const char* JSON_CONFIG_ELEMENT_CONFIG_FILED = "element_config";
constexpr const char* JSON_CONFIG_PORTS_CONFIG_FILED = "ports";
constexpr const char* JSON_CONFIG_INPUT_CONFIG_FILED = "input";
constexpr const char* JSON_CONFIG_OUTPUT_CONFIG_FILED = "output";
constexpr const char* JSON_CONFIG_ELEMENT_IS_SINK_FILED = "is_sink";
constexpr const char* JSON_CONFIG_ELEMENT_IS_SRC_FILED = "is_src";
constexpr const char* JSON_CONFIG_CONNECTION_FILED = "connections";
constexpr const char* JSON_CONFIG_URLS_FILED = "urls";
constexpr const char* JSON_CONFIG_PORT_ID_FILED = "port_id";
constexpr const char* JSON_CONFIG_SRC_ID_FILED = "src_id";
constexpr const char* JSON_CONFIG_SRC_PORT_FILED = "src_port";
constexpr const char* JSON_CONFIG_DST_ID_FILED = "dst_id";
constexpr const char* JSON_CONFIG_DST_PORT_FILED = "dst_port";

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
  ::logInit("debug", "");

  std::mutex mtx;
  std::condition_variable cv;

  sophon_stream::Clocker clocker;
  std::atomic_uint32_t frameCount(0);
  std::atomic_int32_t finishedChannelCount(0);

  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();
  std::ifstream istream, elem_stream;
  nlohmann::json engine_json, config;
  std::string engine_config_file = "../usecase/json/yolov5/engine_config.json";
  std::string config_file = "../usecase/json/yolov5/config_new.json";

  // 初始化engine参数，包括num_graphs, num_channels_per_graph, urls
  istream.open(engine_config_file);
  assert(istream.is_open());
  istream >> engine_json;
  istream.close();
  int num_graphs = engine_json.find(JSON_CONFIG_NUM_GRAPHS_FILED)->get<int>();
  int num_channels_per_graph =
      engine_json.find(JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED)->get<int>();
  int num_channels = num_graphs * num_channels_per_graph;
  std::vector<std::string> urls;
  auto urls_it = engine_json.find(JSON_CONFIG_URLS_FILED);
  assert(urls_it->size() == num_graphs);
  for (auto& url_it : *urls_it) urls.push_back(url_it.get<std::string>());
  int graph_url_idx = 0;
  bool download_image =
      engine_json.find(JSON_CONFIG_DOWNLOAD_IMAGE_FILED)->get<bool>();
  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(config_file);
  assert(istream.is_open());
  istream >> config;
  istream.close();

  if (download_image) {
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

  std::vector<std::string> classnames;
  std::ifstream ifs("../coco.names");
  if (ifs.is_open()) {
    std::string line;
    while (std::getline(ifs, line)) {
      line = line.substr(0, line.length() - 1);
      classnames.push_back(line);
    }
  }

  for (auto& graph_it : config) {
    if (graph_url_idx == num_graphs) break;
    nlohmann::json graphConfigure;
    nlohmann::json elementsConfigure;
    std::pair<int, int> decode_id_port = {-1, -1};  // src_port
    std::pair<int, int> stop_id_port = {-1, -1};    // sink_port

    int graph_id = graph_it.find(JSON_CONFIG_GRAPH_ID_FILED)->get<int>();
    graphConfigure["graph_id"] = graph_id;
    std::string graph_name =
        graph_it.find(JSON_CONFIG_GRAPH_NAME_FILED)->get<std::string>();
    auto elements_it = graph_it.find(JSON_CONFIG_ELEMENTS_FILED);

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
          if (decode_id_port.first != -1) {
            IVS_ERROR("Too many src element");
            abort();
          }
          decode_id_port = {element["id"],
                            input.find(JSON_CONFIG_PORT_ID_FILED)->get<int>()};
        }
      }
      for (auto& output : *output_it) {
        if (output.find(JSON_CONFIG_ELEMENT_IS_SINK_FILED)->get<bool>()) {
          if (stop_id_port.first != -1) {
            IVS_ERROR("Too many sink element");
            abort();
          }
          stop_id_port = {element["id"],
                          output.find(JSON_CONFIG_PORT_ID_FILED)->get<int>()};
          element["is_sink"] = true;
        }
      }

      elementsConfigure.push_back(element);
      elem_stream.close();
    }
    graphConfigure["elements"] = elementsConfigure;

    auto connect_it = graph_it.find(JSON_CONFIG_CONNECTION_FILED);
    for (auto connect_config : *connect_it) {
      int src_id = connect_config.find(JSON_CONFIG_SRC_ID_FILED)->get<int>();
      int src_port =
          connect_config.find(JSON_CONFIG_SRC_PORT_FILED)->get<int>();
      int dst_id = connect_config.find(JSON_CONFIG_DST_ID_FILED)->get<int>();
      int dst_port =
          connect_config.find(JSON_CONFIG_DST_PORT_FILED)->get<int>();
      graphConfigure["connections"].push_back(
          makeConnectConfig(src_id, src_port, dst_id, dst_port));
    }

    engine.addGraph(graphConfigure.dump());

    engine.setStopHandler(
        graph_id, stop_id_port.first, stop_id_port.second,
        [&](std::shared_ptr<void> data) {
          // write stop data handler here
          auto objectMetadata =
              std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(
                  data);
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
          if (download_image) {
            int width = objectMetadata->mFrame->mWidth;
            int height = objectMetadata->mFrame->mHeight;
            bm_image image = *objectMetadata->mFrame->mSpData;
            bm_image imageStorage;
            bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                            FORMAT_YUV420P, image.data_type, &imageStorage);
            bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1,
                                       &image, &imageStorage);
            for (auto subObj : objectMetadata->mSubObjectMetadatas) {
              // draw image
              draw_bmcv(objectMetadata->mFrame->mHandle,
                        subObj->mDetectedObjectMetadata->mClassify, classnames,
                        subObj->mDetectedObjectMetadata->mScores[0],
                        subObj->mDetectedObjectMetadata->mBox.mX,
                        subObj->mDetectedObjectMetadata->mBox.mY,
                        subObj->mDetectedObjectMetadata->mBox.mWidth,
                        subObj->mDetectedObjectMetadata->mBox.mHeight,
                        imageStorage, true);
            }
            // save image
            void* jpeg_data = NULL;
            size_t out_size = 0;
            int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                          &imageStorage, &jpeg_data, &out_size);
            if (ret == BM_SUCCESS) {
              std::string img_file =
                  "./results/" +
                  std::to_string(objectMetadata->mFrame->mChannelId) + "_" +
                  std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
              FILE* fp = fopen(img_file.c_str(), "wb");
              fwrite(jpeg_data, out_size, 1, fp);
              fclose(fp);
            }
            free(jpeg_data);
            bm_image_destroy(imageStorage);
          }
        });

    for (int channel_id = 0; channel_id < num_channels_per_graph;
         ++channel_id) {
      std::string url = urls[graph_url_idx];
      nlohmann::json decodeConfigure;
      decodeConfigure["channel_id"] = channel_id;
      decodeConfigure["url"] = url;
      decodeConfigure["resize_rate"] = 2.0f;
      decodeConfigure["timeout"] = 0;
      decodeConfigure["source_type"] = 0;
      decodeConfigure["multimedia_name"] = "decode_picture";
      decodeConfigure["reopen_times"] = -1;

      auto channelTask =
          std::make_shared<sophon_stream::element::ChannelTask>();
      channelTask->request.operation =
          sophon_stream::element::ChannelOperateRequest::ChannelOperate::START;
      channelTask->request.channelId = channel_id;
      channelTask->request.json = decodeConfigure.dump();
      sophon_stream::common::ErrorCode errorCode = engine.pushInputData(
          graph_id, decode_id_port.first, decode_id_port.second,
          std::static_pointer_cast<void>(channelTask),
          std::chrono::milliseconds(200));
    }
    ++graph_url_idx;
  }
  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for (int i = 0; i < num_graphs; i++) {
    std::cout << "graph stop" << std::endl;
    engine.stop(i);
  }
  long totalCost = clocker.tell_us();
  std::cout << " total time cost " << totalCost << " us." << std::endl;
  double fps = static_cast<double>(frameCount) / totalCost;
  std::cout << "frame count is " << frameCount << " | fps is " << fps * 1000000
            << " fps." << std::endl;
}