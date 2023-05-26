#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "element/MandatoryLink.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/type_trans.hpp"
#include "config.h"
#include <fstream>
#include "common/Clocker.h"
#include <opencv2/opencv.hpp>

#define DECODE_ID 5000
#define PRE_ID 5001
#define YOLO_ID 5002
#define POST_ID 5003
#define ENCODE_ID 5006
#define REPORT_ID 5555

const std::vector<std::vector<int>> colors = {{255, 0, 0}, {255, 85, 0}, {255, 170, 0}, {255, 255, 0}, {170, 255, 0}, {85, 255, 0}, {0, 255, 0}, {0, 255, 85}, {0, 255, 170}, {0, 255, 255}, {0, 170, 255}, {0, 85, 255}, {0, 0, 255}, {85, 0, 255}, {170, 0, 255}, {255, 0, 255}, {255, 0, 170}, {255, 0, 85}, {255, 0, 0}, {255, 0, 255}, {255, 85, 255}, {255, 170, 255}, {255, 255, 255}, {170, 255, 255}, {85, 255, 255}};

void draw_bmcv(bm_handle_t &handle, int classId, const std::vector<std::string> &class_names,
               float conf, int left, int top, int width, int height, bm_image &frame, bool put_text_flag) // Draw the predicted bounding box
{

  int colors_num = colors.size();
  // Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << "," << rect.crop_h << std::endl;
  bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, colors[classId % colors_num][0], colors[classId % colors_num][1], colors[classId % colors_num][2]);

  if (put_text_flag)
  {
    // Get the label for the class name and its confidence
    std::string label = class_names[classId] + ":" + cv::format("%.2f", conf);
    bmcv_point_t org = {left, top};
    bmcv_color_t color = {colors[classId % colors_num][0], colors[classId % colors_num][1], colors[classId % colors_num][2]};
    int thickness = 2;
    float fontScale = 2;
    if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org, color, fontScale, thickness))
    {
      std::cout << "bmcv put text error !!!" << std::endl;
    }
  }
}

/**
@brief SophonYoloX集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值

@UnitCase_Name
usecaseYoloX

@UnitCase_Description
依次经过解码、yoloX检测、编码、输出Element，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
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

#define MAX_GRAPH 1
#define DOWNLOAD_IMAGE 1
TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph)
{
#if DOWNLOAD_IMAGE
  const char * dir_path = "./results";
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

  std::string coco_file = "../coco.names";
  std::vector<std::string> coco_classnames;
  std::ifstream ifs(coco_file);
  if (ifs.is_open())
  {
    std::string line;
    while (std::getline(ifs, line))
    {
      line = line.substr(0, line.length() - 1);
      coco_classnames.push_back(line);
    }
  }


  ::logInit("debug", "", "");
  auto &engine = sophon_stream::framework::SingletonEngine::getInstance();

  std::atomic_int32_t graph_cnt(0);
  std::mutex mtx;
  std::condition_variable cv;
  sophon_stream::Clocker clocker;
  std::atomic_uint32_t frameCount(0);

  for (int i = 0; i < MAX_GRAPH; i++)
  {
    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = i + 1;
    nlohmann::json ElementsConfigure;

    std::ifstream istream;
    nlohmann::json decoder, pre, action, post, encoder, reporter;

    istream.open("../usecase/json/yoloX/Decoder.json");
    assert(istream.is_open());
    istream >> decoder;
    decoder.at("id") = DECODE_ID;
    ElementsConfigure.push_back(decoder);
    istream.close();

    istream.open("../usecase/json/yoloX/Pre.json");
    assert(istream.is_open());
    istream >> pre;
    pre.at("id") = PRE_ID;
    ElementsConfigure.push_back(pre);
    istream.close();

    istream.open("../usecase/json/yoloX/Action.json");
    // istream.open("../usecase/json/yoloX/Action_cls7.json");
    assert(istream.is_open());
    istream >> action;
    action.at("id") = YOLO_ID;
    ElementsConfigure.push_back(action);
    istream.close();

    istream.open("../usecase/json/yoloX/Post.json");
    assert(istream.is_open());
    istream >> post;
    post.at("id") = POST_ID;
    ElementsConfigure.push_back(post);
    istream.close();

    // istream.open("../usecase/json/yoloX/Encoder.json");
    // assert(istream.is_open());
    // istream >> encoder;
    // encoder.at("id") = ENCODE_ID;
    // ElementsConfigure.push_back(encoder);
    // istream.close();

    // istream.open("../usecase/json/yoloX/Reporter.json");
    // assert(istream.is_open());
    // istream >> reporter;
    // reporter.at("id") = REPORT_ID;
    // ElementsConfigure.push_back(reporter);
    // istream.close();

    graphConfigure["elements"] = ElementsConfigure;
    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID, 0, PRE_ID, 0));
    graphConfigure["connections"].push_back(makeConnectConfig(PRE_ID, 0, YOLO_ID, 0));
    graphConfigure["connections"].push_back(makeConnectConfig(YOLO_ID, 0, POST_ID, 0));
    // graphConfigure["connections"].push_back(makeConnectConfig(POST_ID, 0, REPORT_ID, 0));

    engine.addGraph(graphConfigure.dump());

    engine.setDataHandler(i + 1, POST_ID, 0, [&](std::shared_ptr<void> data)
                          {
                            IVS_DEBUG("data output 111111111111111");
                            auto objectMetadata = std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
                            if (objectMetadata == nullptr)
                              return;
                            frameCount++;
                            if (objectMetadata->mFrame->mEndOfStream)
                            {
                              graph_cnt++;
                              if (graph_cnt == MAX_GRAPH)
                              {
                                cv.notify_one();
                              }
                              return;
                            }
#if DOWNLOAD_IMAGE
                            int width = objectMetadata->mFrame->mWidth;
                            int height = objectMetadata->mFrame->mHeight;

                            // 转成bm_image
                            bm_image image = *objectMetadata->mFrame->mSpData;
                            bm_image imageStorage;
                            bm_image_create(objectMetadata->mFrame->mHandle, height, width, FORMAT_YUV420P, image.data_type, &imageStorage);
                            bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image, &imageStorage);
                            // bm_image_destroy(image);

                            for (auto subObj : objectMetadata->mSubObjectMetadatas)
                            {
                              // draw image
                              draw_bmcv(objectMetadata->mFrame->mHandle, subObj->mDetectedObjectMetadata->mClassify, coco_classnames,
                                        subObj->mDetectedObjectMetadata->mScores[0], subObj->mDetectedObjectMetadata->mBox.mX,
                                        subObj->mDetectedObjectMetadata->mBox.mY, subObj->mDetectedObjectMetadata->mBox.mWidth,
                                        subObj->mDetectedObjectMetadata->mBox.mHeight, imageStorage, true);
                            }
                            // save image
                            void *jpeg_data = NULL;
                            size_t out_size = 0;
                            int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1, &imageStorage, &jpeg_data, &out_size);
                            if (ret == BM_SUCCESS)
                            {
                              std::string img_file = "./results/" + std::to_string(objectMetadata->mFrame->mChannelId) + "_" + std::to_string(frameCount) + ".jpg";
                              FILE *fp = fopen(img_file.c_str(), "wb");
                              fwrite(jpeg_data, out_size, 1, fp);
                              fclose(fp);
                            }
                            free(jpeg_data);
                            bm_image_destroy(imageStorage);

#endif
                          });

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test_car_person_1080P.avi";
    // decodeConfigure["url"] = "../test/13.mp4";
    // decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;

    auto channelTask = std::make_shared<sophon_stream::element::ChannelTask>();
    channelTask->request.operation = sophon_stream::element::ChannelOperateRequest::ChannelOperate::START;
    channelTask->request.json = decodeConfigure.dump();
    sophon_stream::common::ErrorCode errorCode = engine.sendData(i + 1,
                                                                 DECODE_ID,
                                                                 0,
                                                                 std::static_pointer_cast<void>(channelTask),
                                                                 std::chrono::milliseconds(200));
  }

  {
    std::unique_lock<std::mutex> uq(mtx);
    cv.wait(uq);
  }
  for(int i=0;i<MAX_GRAPH;i++){
    std::cout << "graph stop" << std::endl;
    engine.stop(i+1);
  }
  long totalCost = clocker.tell_us();
  std::cout << " total time cost " << totalCost << " us." << std::endl;
  double fps = static_cast<double>(frameCount) / totalCost;
  std::cout << "frame count is " << frameCount << " | fps is " << fps * 1000000 << " fps." << std::endl;
}
