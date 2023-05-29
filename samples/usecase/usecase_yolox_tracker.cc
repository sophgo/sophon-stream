#include <sys/stat.h>

#include <fstream>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include "common/Clocker.h"
#include "common/ErrorCode.h"
#include "common/logger.h"
#include "common/ObjectMetadata.h"
#include "common/type_trans.hpp"
#include "DecoderElement.h"
#include "engine.h"
#include "gtest/gtest.h"

#include "config.h"

#define DECODE_ID 5000
#define PRE_ID 5001
#define YOLO_ID 5002
#define POST_ID 5003
#define TRACK_ID 5004

#define MAX_GRAPH 1
#define DOWNLOAD_IMAGE 1

const std::vector<std::vector<int>> colors = {
    {255, 0, 0},    {255, 85, 0},    {255, 170, 0},   {255, 255, 0},
    {170, 255, 0},  {85, 255, 0},    {0, 255, 0},     {0, 255, 85},
    {0, 255, 170},  {0, 255, 255},   {0, 170, 255},   {0, 85, 255},
    {0, 0, 255},    {85, 0, 255},    {170, 0, 255},   {255, 0, 255},
    {255, 0, 170},  {255, 0, 85},    {255, 0, 0},     {255, 0, 255},
    {255, 85, 255}, {255, 170, 255}, {255, 255, 255}, {170, 255, 255},
    {85, 255, 255}};

void draw_bmcv(bm_handle_t& handle, int classId, std::string track_id,
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
    // Get the label for the class name and its confidence
    std::string label = class_names[classId] + ":" + cv::format("%.2f", conf);
    bmcv_point_t org = {left, top};
    bmcv_color_t color = {colors[classId % colors_num][0],
                          colors[classId % colors_num][1],
                          colors[classId % colors_num][2]};
    int thickness = 2;
    float fontScale = 2;
    // bmcv_image_put_text(handle, frame, label.c_str(), org, color, fontScale,
    // thickness);
    org = {left + width, top};
    bmcv_image_put_text(handle, frame, track_id.c_str(), org, color, fontScale,
                        thickness);
  }
}

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
#if DOWNLOAD_IMAGE
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
#endif

  logInit("debug", "");
  std::string coco_file = "../coco.names";
  std::vector<std::string> coco_classnames;
  std::ifstream ifs(coco_file);
  if (ifs.is_open()) {
    std::string line;
    while (std::getline(ifs, line)) {
      line = line.substr(0, line.length() - 1);
      coco_classnames.push_back(line);
    }
  }
  auto& engine = sophon_stream::framework::SingletonEngine::getInstance();

  std::atomic_int32_t graph_cnt(0);
  std::mutex mtx;
  std::condition_variable cv;
  sophon_stream::Clocker clocker;
  std::atomic_uint32_t frameCount(0);

  for (int i = 0; i < MAX_GRAPH; i++) {
    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = i + 1;
    nlohmann::json ElementsConfigure;

    std::ifstream istream;
    nlohmann::json decoder, pre, action, post, tracker;

    istream.open("../usecase/json/yolox_tracker/Decoder.json");
    assert(istream.is_open());
    istream >> decoder;
    decoder.at("id") = DECODE_ID;
    ElementsConfigure.push_back(decoder);
    istream.close();

    istream.open("../usecase/json/yolox_tracker/Pre.json");
    assert(istream.is_open());
    istream >> pre;
    pre.at("id") = PRE_ID;
    ElementsConfigure.push_back(pre);
    istream.close();

    istream.open("../usecase/json/yolox_tracker/Action.json");
    assert(istream.is_open());
    istream >> action;
    action.at("id") = YOLO_ID;
    ElementsConfigure.push_back(action);
    istream.close();

    istream.open("../usecase/json/yolox_tracker/Post.json");
    assert(istream.is_open());
    istream >> post;
    post.at("id") = POST_ID;
    ElementsConfigure.push_back(post);
    istream.close();

    istream.open("../usecase/json/yolox_tracker/Tracker.json");
    assert(istream.is_open());
    istream >> tracker;
    tracker.at("id") = TRACK_ID;
    ElementsConfigure.push_back(tracker);
    istream.close();

    // ElementsConfigure.push_back(makeDecoderElementConfig(DECODE_ID,
    // "decoder_element", "sophgo", 0, 1, 0, false, 1,
    // "../lib/libmultiMediaApi.so"));
    // ElementsConfigure.push_back(makeElementConfig(REPORT_ID,"report_element","host",0,1,0,false,1,
    // {}));

    // nlohmann::json yolov5Json =
    // makeAlgorithmConfig("../lib/libalgorithmApi.so","cocoDetect","Yolov5",
    //     { "../models/yolov5s_int8_4b.bmodel" },
    //     // {"../models/data/model_1684x/yolo"},
    //     1, { "input.1" }, { 1 }, {{3, 640, 640}},  {"147","148","149"},
    //     {3}, {{3,80,80,85},{3,40,40,85},{3,20,20,85}},
    //     { 0.5,0.5 },coco_classnames.size(),coco_classnames);

    // ElementsConfigure.push_back(makeElementConfig(YOLO_ID, "action_element",
    // "sophgo", 0, 1, 200, false, 4, {yolov5Json})); nlohmann::json encodeJson
    // = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    // ElementsConfigure.push_back(makeElementConfig(ENCODE_ID,"action_element","host",0,1,200,true,1,
    // {encodeJson}));

    // nlohmann::json trackJson =
    // makeTrackerConfig("../lib/libalgorithmApi.so","trackerSort","tracker_sort",
    //         1, 1, 0.25f, 20, 3, 4, 3000000,400.0f, 0.7f, 1.3f,
    //         640.0f,360.0f, 10.0f, 19.0f, 112.0f );
    // ElementsConfigure.push_back(makeElementConfig(TRACK_ID,
    // "tracker_element", "sophgo", 0, 1, 200, false, 1, {trackJson}));

    graphConfigure["elements"] = ElementsConfigure;

    graphConfigure["connections"].push_back(
        makeConnectConfig(DECODE_ID, 0, PRE_ID, 0));
    graphConfigure["connections"].push_back(
        makeConnectConfig(PRE_ID, 0, YOLO_ID, 0));
    graphConfigure["connections"].push_back(
        makeConnectConfig(YOLO_ID, 0, POST_ID, 0));
    graphConfigure["connections"].push_back(
        makeConnectConfig(POST_ID, 0, TRACK_ID, 0));

    engine.addGraph(graphConfigure.dump());
    engine.setDataHandler(i + 1, TRACK_ID, 0, [&](std::shared_ptr<void> data) {
      auto objectMetadata =
          std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
      if (objectMetadata == nullptr || objectMetadata->mFrame == nullptr)
        return;
      frameCount++;
      if (objectMetadata->mFrame->mEndOfStream) {
        graph_cnt++;
        if (graph_cnt == MAX_GRAPH) {
          cv.notify_one();
        }
        return;
      }
#if DOWNLOAD_IMAGE
      if (objectMetadata->mFrame && objectMetadata->mFrame->mSpData) {
        int width = objectMetadata->mFrame->mWidth;
        int height = objectMetadata->mFrame->mHeight;
        bm_image image = *objectMetadata->mFrame->mSpData;

        bm_image img;
        bm_image_create(objectMetadata->mFrame->mHandle, height, width,
                        FORMAT_YUV420P, image.data_type, &img);
        bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                                   &img);

        for (auto subObj : objectMetadata->mSubObjectMetadatas) {
          draw_bmcv(objectMetadata->mFrame->mHandle,
                    subObj->mDetectedObjectMetadata->mClassify,
                    std::to_string(subObj->mTrackedObjectMetadata->mTrackId),
                    coco_classnames,
                    subObj->mDetectedObjectMetadata->mScores[0],
                    subObj->mDetectedObjectMetadata->mBox.mX,
                    subObj->mDetectedObjectMetadata->mBox.mY,
                    subObj->mDetectedObjectMetadata->mBox.mWidth,
                    subObj->mDetectedObjectMetadata->mBox.mHeight, img, true);
        }
        void* jpeg_data = NULL;
        size_t out_size = 0;
        int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1, &img,
                                      &jpeg_data, &out_size);
        if (ret == BM_SUCCESS) {
          std::string img_file = "./results/tracker.jpg";
          FILE* fp = fopen(img_file.c_str(), "wb");
          fwrite(jpeg_data, out_size, 1, fp);
          fclose(fp);
        }
        free(jpeg_data);
        bm_image_destroy(img);
      }
#endif
    });
    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test_car_person_1080P.avi";
    // decodeConfigure["url"] = "../test/out.avi";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["source_type"] = 0;
    decodeConfigure["reopen_times"] = -1;

    auto channelTask =
        std::make_shared<sophon_stream::element::ChannelTask>();
    channelTask->request.operation =
        sophon_stream::element::ChannelOperateRequest::ChannelOperate::START;
    channelTask->request.json = decodeConfigure.dump();
    sophon_stream::common::ErrorCode errorCode = engine.sendData(
        i + 1, DECODE_ID, 0, std::static_pointer_cast<void>(channelTask),
        std::chrono::milliseconds(200));
  }

  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for (int i = 0; i < MAX_GRAPH; i++) {
    std::cout << "graph stop" << std::endl;
    engine.stop(i + 1);
  }
  long totalCost = clocker.tell_us();
  std::cout << " total time cost " << totalCost << " us." << std::endl;
  double fps = static_cast<double>(frameCount) / totalCost;
  std::cout << "frame count is " << frameCount << " | fps is " << fps * 1000000
            << " fps." << std::endl;
}