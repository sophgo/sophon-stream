#include <sys/stat.h>

#include <codecvt>
#include <fstream>
#include <locale>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "channel.h"
#include "common/clocker.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "common/profiler.h"
#include "cvUniText.h"
#include "engine.h"
#include "init_engine.h"

const std::vector<std::vector<int>> colors = {
    {0, 0, 0},    {128, 0, 0},   {0, 128, 0},    {128, 128, 0},
    {0, 0, 128},  {128, 0, 128}, {0, 128, 128},  {128, 128, 128},
    {64, 0, 0},   {192, 0, 0},   {64, 128, 0},   {192, 128, 0},
    {64, 0, 128}, {192, 0, 128}, {64, 128, 128}, {192, 128, 128},
    {0, 64, 0},   {128, 64, 0},  {0, 192, 0},    {128, 192, 0},
    {0, 64, 128}};

void draw_bmcv(bm_handle_t& handle, std::string& licensePlate, int left,
               int top, int width, int height, bm_image& frame,
               bool put_text_flag)  // Draw the predicted bounding box
{
  // Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << ","
            << rect.crop_h << std::endl;
  std::vector<int> color = colors[0];
  bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, color[0], color[1],
                            color[2]);
  if (put_text_flag) {
    bmcv_point_t org = {left, top};
    bmcv_color_t bmcv_color = {color[0], color[1], color[2]};
    int thickness = 2;
    float fontScale = 2;

    if (BM_SUCCESS != bmcv_image_put_text(handle, frame, licensePlate.c_str(),
                                          org, bmcv_color, fontScale,
                                          thickness)) {
      std::cout << "bmcv put text error !!!" << std::endl;
    }
  }
}

typedef struct demo_config_ {
  int num_graphs;
  int num_channels_per_graph;
  nlohmann::json channel_config;
  bool download_image;
  std::string engine_config_file;
  std::vector<std::string> class_names;
} demo_config;

constexpr const char* JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED =
    "num_channels_per_graph";
constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";
constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CLASS_NAMES_FILED = "class_names";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FILED = "channel";

// parse json
demo_config parse_demo_json(std::string& json_path) {
  std::ifstream istream;
  istream.open(json_path);
  assert(istream.is_open());
  nlohmann::json demo_json;
  istream >> demo_json;
  istream.close();

  demo_config config;

  config.num_channels_per_graph =
      demo_json.find(JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED)->get<int>();

  auto channel_config_it = demo_json.find(JSON_CONFIG_CHANNEL_CONFIG_FILED);
  config.channel_config = *channel_config_it;

  config.download_image =
      demo_json.find(JSON_CONFIG_DOWNLOAD_IMAGE_FILED)->get<bool>();
  config.engine_config_file =
      demo_json.find(JSON_CONFIG_ENGINE_CONFIG_PATH_FILED)->get<std::string>();
  std::string class_names_file =
      demo_json.find(JSON_CONFIG_CLASS_NAMES_FILED)->get<std::string>();

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
    istream.open(class_names_file);
    assert(istream.is_open());
    std::string line;
    while (std::getline(istream, line)) {
      line = line.substr(0, line.length());
      config.class_names.push_back(line);
    }
    istream.close();
  }

  return config;
}

// build graph
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
  std::string licensePlate_config_file =
      "../config/license_plate_recognition_demo.json";
  demo_config licensePlate_json = parse_demo_json(licensePlate_config_file);

  // 启动每个graph, graph之间没有联系,可以是完全不同的配置
  istream.open(licensePlate_json.engine_config_file);
  assert(istream.is_open());
  istream >> engine_json;
  istream.close();

  licensePlate_json.num_graphs = engine_json.size();
  int num_channels =
      licensePlate_json.num_channels_per_graph * licensePlate_json.num_graphs;
  ::sophon_stream::common::FpsProfiler fpsProfiler("fps_licensePlate_demo",
                                                   100);
  uni_text::UniText uniText("../data/wqy-microhei.ttc", 22);
  // 统计帧数和绘图
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
    if (licensePlate_json.download_image) {
      int width = objectMetadata->mFrame->mWidth;
      int height = objectMetadata->mFrame->mHeight;
      bm_image image = *objectMetadata->mFrame->mSpData;
      bm_image imageStorage;
      bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                      FORMAT_YUV420P, image.data_type, &imageStorage);
      bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                                 &imageStorage);
      cv::Mat img;
      cv::bmcv::toMAT(&imageStorage, img);
      // get license plate, and draw pics
      if (objectMetadata->mSubObjectMetadatas.size() > 0) {
        for (auto subObj : objectMetadata->mSubObjectMetadatas) {
          IVS_WARN("get recognized license plate datas from yolo and lprnet");
          int subId = subObj->mSubId;
          auto reconizedObj = subObj->mRecognizedObjectMetadatas[0];
          auto detObj = objectMetadata->mDetectedObjectMetadatas[subId];

          // get license plate
          std::string oriLabel = reconizedObj->mLabelName;

          // draw pics for wide str
          draw_bmcv(objectMetadata->mFrame->mHandle, oriLabel, detObj->mBox.mX,
                    detObj->mBox.mY, detObj->mBox.mWidth, detObj->mBox.mHeight,
                    imageStorage, false);

          uniText.PutText(img, oriLabel,
                          cv::Point(detObj->mBox.mX, detObj->mBox.mY),
                          cv::Scalar(0, 0, 255), false);
        }
        std::string img_file =
            "./results/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "_" + std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
        cv::imwrite(img_file, img);
        bm_image_destroy(imageStorage);
      }
    } else {
      IVS_WARN(
          "CANNOT get recognized license plate datas from yolo and lprnet");
    }
    fpsProfiler.add(1);
  };

  std::map<int, std::pair<int, int>> graph_src_id_port_map;

  // 解析engine.json, 并初始化
  init_engine(engine, engine_json, sinkHandler, graph_src_id_port_map);

  // 发送解码信号
  for (auto graph_id : engine.getGraphIds()) {
    for (int channel_id = 0;
         channel_id < licensePlate_json.num_channels_per_graph; ++channel_id) {
      nlohmann::json channel_config = licensePlate_json.channel_config;
      channel_config["channel_id"] = channel_id;
      auto channelTask =
          std::make_shared<sophon_stream::element::decode::ChannelTask>();

      channelTask->request.operation = sophon_stream::element::decode::
          ChannelOperateRequest::ChannelOperate::START;

      channelTask->request.json = channel_config.dump();
      std::pair<int, int> src_id_port = graph_src_id_port_map[graph_id];
      sophon_stream::common::ErrorCode errorCode =
          engine.pushSourceData(graph_id, src_id_port.first, src_id_port.second,
                                std::static_pointer_cast<void>(channelTask));
    }
  }

  // 等待运行结束，释放资源，统计fps等信息
  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }

  for (int i = 0; i < licensePlate_json.num_graphs; i++) {
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