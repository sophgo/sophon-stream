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
  // cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom),
  // cv::Scalar(0, 0, 255), 3);

  if (put_text_flag) {
    // Get the label for the class name and its confidence
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

/**
@brief SophonYolov5集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值

@UnitCase_Name
usecaseYolov5

@UnitCase_Description
依次经过解码、yolov5检测、编码、输出Element，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
具体先给各个Element赋值，定义pipeline中各个Element的先后连接顺序，然后添加graph并发送数据，接受数据并实时显示结果

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestMultiAlgorithmGraph, MultiAlgorithmGraph

@UnitCase_ExpectedResult
播放视频，在每一帧都会进行检测并将对应的box绘制在相应位置，播放结束程序可以正常退出

*/
constexpr const char* JSON_CONFIG_NUM_GRAPHS_FILED = "num_graphs";
constexpr const char* JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED =
    "num_channels_per_graph";
constexpr const char* JSON_CONFIG_DOWNLOAD_IMAGE_FILED = "download_image";
constexpr const char* JSON_CONFIG_ELEMENTS_ID_FILED = "elements_id";
constexpr const char* JSON_CONFIG_CONNECTION_FILED = "connection";
constexpr const char* JSON_CONFIG_URL_FILED = "url";
constexpr const char* JSON_CONFIG_CLASS_NAMES_FILED = "class_names";
constexpr const char* JSON_CONFIG_SRC_ID_FILED = "src_id";
constexpr const char* JSON_CONFIG_SRC_PORT_FILED = "src_port";
constexpr const char* JSON_CONFIG_DST_ID_FILED = "dst_id";
constexpr const char* JSON_CONFIG_DST_PORT_FILED = "dst_port";
constexpr const char* JSON_CONFIG_STOP_HANDLER_ID_FILED = "stop_handler_id";
constexpr const char* JSON_CONFIG_STOP_HANDLER_PORT_FILED = "stop_handler_port";

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
  std::ifstream istream;
  nlohmann::json config;

  // init
  istream.open("../usecase/json/yolov5/Config.json");
  assert(istream.is_open());
  istream >> config;

  auto num_graphs_it = config.find(JSON_CONFIG_NUM_GRAPHS_FILED);
  auto num_channels_per_graph_it =
      config.find(JSON_CONFIG_NUM_CHANNELS_PER_GRAPH_FILED);
  auto download_image_it = config.find(JSON_CONFIG_DOWNLOAD_IMAGE_FILED);
  auto url_it = config.find(JSON_CONFIG_URL_FILED);
  auto class_names_it = config.find(JSON_CONFIG_CLASS_NAMES_FILED);
  auto stop_handler_id_it = config.find(JSON_CONFIG_STOP_HANDLER_ID_FILED);
  auto stop_handler_port_it = config.find(JSON_CONFIG_STOP_HANDLER_PORT_FILED);

  int num_graphs = num_graphs_it->get<int>();
  int num_channels_per_graph = num_channels_per_graph_it->get<int>();
  int num_channels = num_graphs * num_channels_per_graph;
  bool download_image = download_image_it->get<bool>();
  std::string url = url_it->get<std::string>();
  std::string class_name_file = class_names_it->get<std::string>();
  int stop_handler_id = stop_handler_id_it->get<int>();
  int stop_handler_port = stop_handler_port_it->get<int>();

  std::unordered_map<std::string, int> elements_id;
  auto element_it = config.find(JSON_CONFIG_ELEMENTS_ID_FILED);
  if (element_it == config.end()) {
    IVS_ERROR("Can not find elements in json configure");
    abort();
  }
  for (auto elem : element_it->items()) {
    elements_id.insert({elem.key(), elem.value()});
  }

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
  std::ifstream ifs(class_name_file);
  if (ifs.is_open()) {
    std::string line;
    while (std::getline(ifs, line)) {
      line = line.substr(0, line.length() - 1);
      classnames.push_back(line);
    }
  }

  ::logInit("debug", "");

  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();

  std::atomic_int32_t finishedChannelCount(0);
  std::mutex mtx;
  std::condition_variable cv;
  sophon_stream::Clocker clocker;
  std::atomic_uint32_t frameCount(0);

  for (int i = 0; i < num_graphs; i++) {
    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = i;
    nlohmann::json ElementsConfigure;

    nlohmann::json decoder, pre, action, post;

    for (const std::pair<std::string, int>& kv : elements_id) {
      nlohmann::json elem;
      istream.open("../usecase/json/yolov5/" + kv.first);
      assert(istream.is_open());
      istream >> elem;
      elem.at("id") = kv.second;
      ElementsConfigure.push_back(elem);
      istream.close();
    }
    graphConfigure["elements"] = ElementsConfigure;

    auto connect_it = config.find(JSON_CONFIG_CONNECTION_FILED);
    if (connect_it == config.end() || !connect_it->is_array() ||
        connect_it->empty()) {
      IVS_ERROR("Can not find elements in json configure");
      abort();
    }
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

    engine.setStopHandler(i, stop_handler_id, stop_handler_port, [&](std::shared_ptr<void> data) {
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
      if (download_image) {
        int width = objectMetadata->mFrame->mWidth;
        int height = objectMetadata->mFrame->mHeight;
        bm_image image = *objectMetadata->mFrame->mSpData;
        bm_image imageStorage;
        bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                        FORMAT_YUV420P, image.data_type, &imageStorage);
        bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                                   &imageStorage);
        for (auto subObj : objectMetadata->mSubObjectMetadatas) {
          // draw image
          draw_bmcv(objectMetadata->mFrame->mHandle,
                    subObj->mDetectedObjectMetadata->mClassify, classnames,
                    subObj->mDetectedObjectMetadata->mScores[0],
                    subObj->mDetectedObjectMetadata->mBox.mX,
                    subObj->mDetectedObjectMetadata->mBox.mY,
                    subObj->mDetectedObjectMetadata->mBox.mWidth,
                    subObj->mDetectedObjectMetadata->mBox.mHeight, imageStorage,
                    true);
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

    nlohmann::json decodeConfigure[num_channels_per_graph];
    for (int j = 0; j < num_channels_per_graph; j++) {
      decodeConfigure[j]["channel_id"] = j;
      decodeConfigure[j]["url"] = url;
      decodeConfigure[j]["resize_rate"] = 2.0f;
      decodeConfigure[j]["timeout"] = 0;
      decodeConfigure[j]["source_type"] = 0;
      decodeConfigure[j]["multimedia_name"] = "decode_picture";
      decodeConfigure[j]["reopen_times"] = -1;

      auto channelTask =
          std::make_shared<sophon_stream::element::ChannelTask>();
      channelTask->request.operation =
          sophon_stream::element::ChannelOperateRequest::ChannelOperate::START;
      channelTask->request.channelId = j;
      channelTask->request.json = decodeConfigure[j].dump();
      sophon_stream::common::ErrorCode errorCode = engine.pushInputData(
          i, elements_id["decoder"], 0, std::static_pointer_cast<void>(channelTask),
          std::chrono::milliseconds(200));
    }
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
