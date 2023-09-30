#include <dirent.h>
#include <sys/stat.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "channel.h"
#include "common/clocker.h"
#include "common/common_defs.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "common/profiler.h"
#include "engine.h"
#include "init_engine.h"
using json = nlohmann::json;
using namespace sophon_stream::common;
#define POSE_COLORS_RENDER_CPU                                                 \
  255.f, 0.f, 0.f, 255.f, 85.f, 0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f,     \
      170.f, 255.f, 0.f, 85.f, 255.f, 0.f, 0.f, 255.f, 0.f, 0.f, 255.f, 85.f,  \
      0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f, 170.f, 255.f, 0.f, 85.f,      \
      255.f, 0.f, 0.f, 255.f, 85.f, 0.f, 255.f, 170.f, 0.f, 255.f, 255.f, 0.f, \
      255.f, 255.f, 0.f, 170.f, 255.f, 0.f, 85.f, 255.f, 0.f, 0.f, 255.f, 0.f, \
      255.f, 255.f, 85.f, 255.f, 255.f, 170.f, 255.f, 255.f, 255.f, 255.f,     \
      170.f, 255.f, 255.f, 85.f, 255.f, 255.f

const std::vector<float> POSE_COLORS_RENDER{POSE_COLORS_RENDER_CPU};

const unsigned int POSE_MAX_PEOPLE = 96;

// // Round functions

template <typename T>
inline int intRound(const T a) {
  return int(a + 0.5f);
}

// Max/min functions
template <typename T>
inline T fastMax(const T a, const T b) {
  return (a > b ? a : b);
}

std::vector<unsigned int> getPosePairs(
    PosedObjectMetadata::EModelType model_type) {
  switch (model_type) {
    case PosedObjectMetadata::EModelType::BODY_25:
      return {1,  8,  1,  2,  1,  5,  2,  3,  3,  4,  5,  6,  6,
              7,  8,  9,  9,  10, 10, 11, 8,  12, 12, 13, 13, 14,
              1,  0,  0,  15, 15, 17, 0,  16, 16, 18, 2,  17, 5,
              18, 14, 19, 19, 20, 14, 21, 11, 22, 22, 23, 11, 24};
    case PosedObjectMetadata::EModelType::COCO_18:
      return {1, 2,  1,  5,  2,  3,  3,  4,  5,  6,  6,  7, 1,
              8, 8,  9,  9,  10, 1,  11, 11, 12, 12, 13, 1, 0,
              0, 14, 14, 16, 0,  15, 15, 17, 2,  16, 5,  17};
    default:
      // COCO_18
      return {1, 2,  1,  5,  2,  3,  3,  4,  5,  6,  6,  7, 1,
              8, 8,  9,  9,  10, 1,  11, 11, 12, 12, 13, 1, 0,
              0, 14, 14, 16, 0,  15, 15, 17, 2,  16, 5,  17};
  }
}

void renderKeypointsBmcv(bm_handle_t& handle, bm_image& frame,
                         const std::vector<float>& keypoints,

                         const std::vector<unsigned int>& pairs,
                         const std::vector<float> colors,
                         const float thicknessCircleRatio,
                         const float thicknessLineRatioWRTCircle,
                         const float threshold, float scale) {
  // Get frame channels
  const auto width = frame.width;
  const auto height = frame.height;
  const auto area = width * height;

  // Parameters
  const auto lineType = 8;
  const auto shift = 0;
  const auto numberColors = colors.size();
  const auto thresholdRectangle = 0.1f;

  // Keypoints

  const auto ratioAreas = 1;
  // Size-dependent variables
  const auto thicknessRatio =
      fastMax(intRound(std::sqrt(area) * thicknessCircleRatio * ratioAreas), 1);
  // Negative thickness in cv::circle means that a filled circle is to
  // be drawn.
  const auto thicknessCircle = (ratioAreas > 0.05 ? thicknessRatio : -1);
  const auto thicknessLine =
      2;  // intRound(thicknessRatio * thicknessLineRatioWRTCircle);
  const auto radius = thicknessRatio / 2;

  // Draw lines
  for (auto pair = 0u; pair < pairs.size(); pair += 2) {
    const auto index1 = (pairs[pair]) * 3;
    const auto index2 = (pairs[pair + 1]) * 3;

    if (keypoints[index1 + 2] > threshold &&
        keypoints[index2 + 2] > threshold) {
      const auto colorIndex = pairs[pair + 1] * 3;
      bmcv_color_t color = {colors[(colorIndex + 2) % numberColors],
                            colors[(colorIndex + 1) % numberColors],
                            colors[(colorIndex + 0) % numberColors]};
      bmcv_point_t start = {intRound(keypoints[index1] * scale),
                            intRound(keypoints[index1 + 1] * scale)};
      bmcv_point_t end = {intRound(keypoints[index2] * scale),
                          intRound(keypoints[index2 + 1] * scale)};

      if (BM_SUCCESS != bmcv_image_draw_lines(handle, frame, &start, &end, 1,
                                              color, thicknessLine)) {
        std::cout << "bmcv draw lines error !!!" << std::endl;
      }
    }
  }
}

void renderPoseKeypointsBmcv(bm_handle_t& handle, bm_image& frame,
                             const std::vector<float>& poseKeypoints,
                             const float renderThreshold, float scale,
                             PosedObjectMetadata::EModelType modelType,
                             const bool blendOriginalFrame) {
  // Background
  // if (!blendOriginalFrame)
  //     frame.setTo(0.f); // [0-255]

  // Parameters
  const auto thicknessCircleRatio = 1.f / 75.f;
  const auto thicknessLineRatioWRTCircle = 0.75f;
  const auto& pairs = getPosePairs(modelType);

  // Render keypoints
  renderKeypointsBmcv(handle, frame, poseKeypoints, pairs, POSE_COLORS_RENDER,
                      thicknessCircleRatio, thicknessLineRatioWRTCircle,
                      renderThreshold, scale);
}
typedef struct demo_config_ {
  int num_graphs;
  int num_channels_per_graph;
  nlohmann::json channel_config;
  bool download_image;
  std::string engine_config_file;
} demo_config;

constexpr const char* JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED =
    "num_channels_per_graph";
constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";
constexpr const char* JSON_CONFIG_ENGINE_CONFIG_PATH_FILED =
    "engine_config_path";
constexpr const char* JSON_CONFIG_CHANNEL_CONFIG_FILED = "channel";

demo_config parse_demo_json(std::string& json_path) {
  std::ifstream istream;
  istream.open(json_path);
  STREAM_CHECK(istream.is_open(), "Please check config file ", json_path,
               " exists.");
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
std::vector<std::string> files_vector;

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
  std::string openpose_config_file = "../config/openpose_demo.json";
  demo_config openpose_json = parse_demo_json(openpose_config_file);

  std::string input =
      openpose_json.channel_config.find("url")->get<std::string>();
  struct stat info;

  if (stat(input.c_str(), &info) != 0) {
    std::cout << "Cannot find input path." << std::endl;
    exit(1);
  }

  if (info.st_mode & S_IFDIR) {
    // get files
    DIR* pDir;
    struct dirent* ptr;
    pDir = opendir(input.c_str());
    while ((ptr = readdir(pDir)) != 0) {
      if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
        files_vector.push_back(ptr->d_name);
      }
    }
    closedir(pDir);
    sort(files_vector.begin(), files_vector.end());
  }

  // 启动每个graph, graph之间没有联系，可以是完全不同的配置
  istream.open(openpose_json.engine_config_file);
  STREAM_CHECK(istream.is_open(), "Please check if engine_config_file ",
               openpose_json.engine_config_file, " exists.");
  istream >> engine_json;
  istream.close();

  openpose_json.num_graphs = engine_json.size();
  int num_channels =
      openpose_json.num_channels_per_graph * openpose_json.num_graphs;
  ::sophon_stream::common::FpsProfiler fpsProfiler("fps_openpose_demo", 100);
  auto sinkHandler = [&](std::shared_ptr<void> data) {
    // write stop data handler here
    auto objectMetadata =
        std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
    if (objectMetadata == nullptr) return;
    frameCount++;
    fpsProfiler.add(1);
    if (objectMetadata->mFrame->mEndOfStream) {
      printf("meet a eof\n");
      finishedChannelCount++;
      if (finishedChannelCount == num_channels) {
        cv.notify_one();
      }
      return;
    }
    if (openpose_json.download_image) {
      int width = objectMetadata->mFrame->mWidth;
      int height = objectMetadata->mFrame->mHeight;
      bm_image image = *objectMetadata->mFrame->mSpData;
      bm_image imageStorage;

      if (image.image_format == 11) {
        bm_image image_aligned;
        bool need_copy = image.width & (64 - 1);
        if (need_copy) {
          int stride1[3], stride2[3];
          bm_image_get_stride(image, stride1);
          stride2[0] = FFALIGN(stride1[0], 64);
          stride2[1] = FFALIGN(stride1[1], 64);
          stride2[2] = FFALIGN(stride1[2], 64);

          bm_image_create(objectMetadata->mFrame->mHandle, image.height,
                          image.width, image.image_format, image.data_type,
                          &image_aligned, stride2);
          bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
          bmcv_copy_to_atrr_t copyToAttr;
          memset(&copyToAttr, 0, sizeof(copyToAttr));
          copyToAttr.start_x = 0;
          copyToAttr.start_y = 0;
          copyToAttr.if_padding = 1;
          bmcv_image_copy_to(objectMetadata->mFrame->mHandle, copyToAttr, image,
                             image_aligned);
        } else {
          image_aligned = image;
        }

        bm_image_create(objectMetadata->mFrame->mHandle, image.height,
                        image.width, FORMAT_YUV420P, image.data_type,
                        &imageStorage);
        bmcv_rect_t crop_rect = {0, 0, image.width, image.height};
        bmcv_image_vpp_convert(objectMetadata->mFrame->mHandle, 1,
                               image_aligned, &imageStorage, &crop_rect);
        if (need_copy) bm_image_destroy(image_aligned);
      }
      for (auto subObj : objectMetadata->mPosedObjectMetadatas) {
        renderPoseKeypointsBmcv(objectMetadata->mFrame->mHandle, imageStorage,
                                subObj->keypoints, 0.05, 1.0, subObj->modeltype,
                                0);
      }
      void* jpeg_data = NULL;
      size_t out_size = 0;
      int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1,
                                    &imageStorage, &jpeg_data, &out_size);

      if (ret == BM_SUCCESS) {
        if (info.st_mode & S_IFDIR) {
          std::string img_file =
              "./results/" +
              std::to_string(objectMetadata->mFrame->mChannelId) + "_" +
              files_vector[objectMetadata->mFrame->mFrameId];
          FILE* fp = fopen(img_file.c_str(), "wb");
          fwrite(jpeg_data, out_size, 1, fp);
          fclose(fp);
        } else {
          std::string img_file =
              "./results/" +
              std::to_string(objectMetadata->mFrame->mChannelId) + "_" +
              std::to_string(objectMetadata->mFrame->mFrameId) + ".jpg";
          FILE* fp = fopen(img_file.c_str(), "wb");
          fwrite(jpeg_data, out_size, 1, fp);
          fclose(fp);
        }
      }
      free(jpeg_data);
      bm_image_destroy(imageStorage);
    }
  };

  std::map<int, std::pair<int, int>> graph_src_id_port_map;
  init_engine(engine, engine_json, sinkHandler, graph_src_id_port_map);

  for (auto graph_id : engine.getGraphIds()) {
    for (int channel_id = 0; channel_id < openpose_json.num_channels_per_graph;
         ++channel_id) {
      nlohmann::json channel_config = openpose_json.channel_config;
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

  for (int i = 0; i < openpose_json.num_graphs; i++) {
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
