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
#include "engine.h"
#include "config.h"
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
constexpr const char* JSON_CONFIG_ELEMENTS_FILED = "elements";
constexpr const char* JSON_CONFIG_CONNECTION_FILED = "connections";

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
  ::logInit("debug", "");

  std::mutex mtx;
  std::condition_variable cv;

  sophon_stream::Clocker clocker;
  std::atomic_uint32_t frameCount(0);
  std::atomic_int32_t finishedChannelCount(0);

  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();
  std::ifstream istream, elem_stream;
  nlohmann::json engine_json;
  std::string yolox_config_file = "../config/yolox.json";

  // 初始化engine参数，包括num_graphs, num_channels_per_graph, urls
  usecase_config yolox_json = parse_usecase_json(yolox_config_file);
  int num_channels = yolox_json.num_channels_per_graph * yolox_json.num_graphs;

  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(yolox_json.engine_config_file);
  assert(istream.is_open());
  istream >> engine_json;
  istream.close();

  int graph_url_idx = 0;
  for (auto& graph_it : engine_json) {
    if (graph_url_idx == yolox_json.num_graphs) break;
    nlohmann::json graphConfigure, elementsConfigure;
    std::pair<int, int> src_id_port = {-1, -1};  // src_port
    std::pair<int, int> sink_id_port = {-1, -1};    // sink_port

    int graph_id = graph_it.find(JSON_CONFIG_GRAPH_ID_FILED)->get<int>();
    graphConfigure["graph_id"] = graph_id;
    auto elements_it = graph_it.find(JSON_CONFIG_ELEMENTS_FILED);
    parse_element_json(elements_it, elementsConfigure, src_id_port, sink_id_port);
    graphConfigure["elements"] = elementsConfigure;
    auto connect_it = graph_it.find(JSON_CONFIG_CONNECTION_FILED);
    parse_connection_json(connect_it, graphConfigure);

    engine.addGraph(graphConfigure.dump());
    engine.setStopHandler(
        graph_id, sink_id_port.first, sink_id_port.second,
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
          if (yolox_json.download_image) {
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
                        subObj->mDetectedObjectMetadata->mClassify, yolox_json.class_names,
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

    for (int channel_id = 0; channel_id < yolox_json.num_channels_per_graph;
         ++channel_id) {
      nlohmann::json decodeConfigure = yolox_json.decodeConfigures[channel_id];
      decodeConfigure["channel_id"] = channel_id;
      auto channelTask =
          std::make_shared<sophon_stream::element::ChannelTask>();
      channelTask->request.operation =
          sophon_stream::element::ChannelOperateRequest::ChannelOperate::START;
      channelTask->request.channelId = channel_id;
      channelTask->request.json = decodeConfigure.dump();
      sophon_stream::common::ErrorCode errorCode = engine.pushInputData(
          graph_id, src_id_port.first, src_id_port.second,
          std::static_pointer_cast<void>(channelTask),
          std::chrono::milliseconds(200));
    }
    ++graph_url_idx;
  }
  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for (int i = 0; i < yolox_json.num_graphs; i++) {
    std::cout << "graph stop" << std::endl;
    engine.stop(i);
  }
  long totalCost = clocker.tell_us();
  std::cout << " total time cost " << totalCost << " us." << std::endl;
  double fps = static_cast<double>(frameCount) / totalCost;
  std::cout << "frame count is " << frameCount << " | fps is " << fps * 1000000
            << " fps." << std::endl;
}