#include <sys/stat.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "channel.h"
#include "common/clocker.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "common/profiler.h"
#include "engine.h"
#include "init_engine.h"

using namespace cv;
using namespace std;

typedef struct demo_config_ {
  int num_graphs;
  int num_channels_per_graph;
  nlohmann::json channel_config;
  bool download_image;
  std::string engine_config_file;
} demo_config;

constexpr const char* JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED =
    "num_channels_per_graph";
constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FILED = "channel";
constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";

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

  config.engine_config_file =
      demo_json.find(JSON_CONFIG_ENGINE_CONFIG_PATH_FILED)->get<std::string>();

  config.download_image =
      demo_json.find(JSON_CONFIG_DOWNLOAD_IMAGE_FILED)->get<bool>();

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
  return config;
}

void draw_all_bmcv(bm_handle_t& handle, int left, int top, int width,
                   int height, bm_image& frame,
                   string label)  // Draw the predicted bounding box
{
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << ","
            << rect.crop_h << std::endl;
  // bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, 255, 0,0);
  bmcv_point_t org = {left, top + 40};
  bmcv_color_t bmcv_color = {255, 0, 0};
  int thickness = 2;
  float fontScale = 1.5;
  if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org,
                                        bmcv_color, fontScale, thickness)) {
    std::cout << "bmcv put text error !!!" << std::endl;
  }
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
  std::string retinaface_config_file =
      "../config/retinaface_distributor_resnet_faiss_converger.json";
  demo_config retinaface_json = parse_demo_json(retinaface_config_file);

  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(retinaface_json.engine_config_file);
  assert(istream.is_open());
  istream >> engine_json;
  istream.close();

  retinaface_json.num_graphs = engine_json.size();
  int num_channels =
      retinaface_json.num_channels_per_graph * retinaface_json.num_graphs;
  ::sophon_stream::common::FpsProfiler fpsProfiler("fps_retinaface_demo", 100);
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

    if (retinaface_json.download_image) {
      int width = objectMetadata->mFrame->mWidth;
      int height = objectMetadata->mFrame->mHeight;
      bm_image image = *objectMetadata->mFrame->mSpData;  // 取出第一张大图

      if (objectMetadata->mSubObjectMetadatas.size() > 0) {
        bm_image imageStorage;
        bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                        FORMAT_YUV420P, image.data_type, &imageStorage);
        bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                                   &imageStorage);
        for (auto subObj : objectMetadata->mSubObjectMetadatas) {
          int subId = subObj->mSubId;
          auto faceObj =
              objectMetadata->mFaceObjectMetadatas[subId];  // 第一张脸
          auto resnetObj =
              subObj->mRecognizedObjectMetadatas[0];  // 第一张脸对应的resnet
          int class_id = subObj->mRecognizedObjectMetadatas[0]->mTopKLabels[0];
          auto label = subObj->mRecognizedObjectMetadatas[0]->mLabelName;

          bmcv_rect_t rect;
          rect.start_x = max(faceObj->left, 0);
          rect.start_y = max(faceObj->top, 0);
          rect.crop_w = max(faceObj->right - faceObj->left + 1, 0);
          rect.crop_h = max(faceObj->bottom - faceObj->top + 1, 0);

          draw_all_bmcv(objectMetadata->mFrame->mHandle, rect.start_x,
                        rect.start_y, rect.crop_w, rect.crop_h, imageStorage,
                        label);

          std::cout << "label:" << label << std::endl;
        }

        std::string filename =
            "./results/" + std::to_string(objectMetadata->mFrame->mChannelId) +
            "-" + std::to_string(objectMetadata->mFrame->mFrameId) + ".bmp";
        bm_image_write_to_bmp(imageStorage, filename.c_str());
        bm_image_destroy(imageStorage);
      }
    }
    fpsProfiler.add(1);
  };

  std::map<int, std::pair<int, int>> graph_src_id_port_map;
  init_engine(engine, engine_json, sinkHandler, graph_src_id_port_map);

  for (auto graph_id : engine.getGraphIds()) {
    for (int channel_id = 0;
         channel_id < retinaface_json.num_channels_per_graph; ++channel_id) {
      nlohmann::json channel_config = retinaface_json.channel_config;
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

  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for (int i = 0; i < retinaface_json.num_graphs; i++) {
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
