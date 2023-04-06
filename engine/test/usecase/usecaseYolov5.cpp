#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "element/MandatoryLink.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/type_trans.hpp"
#include "config.h"
#include <fstream>

#include <opencv2/opencv.hpp>

#define DECODE_ID 5000
#define YOLO_ID 5001
#define ENCODE_ID 5006
#define REPORT_ID 5555

const std::vector<std::vector<int>> colors = {{255, 0, 0}, {255, 85, 0}, {255, 170, 0}, {255, 255, 0}, {170, 255, 0}, \
                {85, 255, 0}, {0, 255, 0}, {0, 255, 85}, {0, 255, 170}, {0, 255, 255}, {0, 170, 255}, {0, 85, 255}, \
                {0, 0, 255}, {85, 0, 255}, {170, 0, 255}, {255, 0, 255}, {255, 0, 170}, {255, 0, 85}, {255, 0, 0},\
                {255, 0, 255}, {255, 85, 255}, {255, 170, 255}, {255, 255, 255}, {170, 255, 255}, {85, 255, 255}};

void draw_bmcv(bm_handle_t &handle, int classId, std::vector<std::string>& class_names,
float conf, int left, int top, int width, int height, bm_image& frame,bool put_text_flag)   // Draw the predicted bounding box
{
  int colors_num = colors.size();
  //Draw a rectangle displaying the bounding box
  bmcv_rect_t rect;
  rect.start_x = left;
  rect.start_y = top;
  rect.crop_w = width;
  rect.crop_h = height;
  std::cout << rect.start_x << "," << rect.start_y << "," << rect.crop_w << "," << rect.crop_h << std::endl;
  bmcv_image_draw_rectangle(handle, frame, 1, &rect, 3, colors[classId % colors_num][0], colors[classId % colors_num][1], colors[classId % colors_num][2]);
  // cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 3);

  if (put_text_flag){
    //Get the label for the class name and its confidence
    std::string label = class_names[classId] + ":" + cv::format("%.2f", conf);
    // Display the label at the top of the bounding box
    // int baseLine;
    // cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    // top = std::max(top, labelSize.height);
    // //rectangle(frame, Point(left, top - int(1.5 * labelSize.height)), Point(left + int(1.5 * labelSize.width), top + baseLine), Scalar(0, 255, 0), FILLED);
    // cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 255, 0), 1);
    bmcv_point_t org = {left, top};
    bmcv_color_t color = {colors[classId % colors_num][0], colors[classId % colors_num][1], colors[classId % colors_num][2]};
    int thickness = 2;
    float fontScale = 2; 
    if (BM_SUCCESS != bmcv_image_put_text(handle, frame, label.c_str(), org, color, fontScale, thickness)) {
      std::cout << "bmcv put text error !!!" << std::endl;   
    }
  }
}



/**
@brief GpuYolov3集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
GpuYolov3_IT_0014

@UnitCase_Name
usecaseGpuYolov3

@UnitCase_Description
依次经过解码、yolov3人脸检测、编码、输出Element，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
具体先给各个Element赋值，定义pipeline中各个Element的先后连接顺序，然后添加graph并发送数据，接受数据并实时显示结果

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestMultiAlgorithmGraph, MultiAlgorithmGraph

@UnitCase_ExpectedResult
播放视频，在每一帧都会检测出人脸并将对应的box绘制在相应位置，播放结束程序可以正常退出

*/

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    std::string coco_file = "../test/coco.names";
    std::vector<std::string> coco_classnames;
    std::ifstream ifs(coco_file);
    if (ifs.is_open()) {
        std::string line;
        while(std::getline(ifs, line)) {
                line = line.substr(0, line.length() - 1);
                coco_classnames.push_back(line);
        }
    }

    ::logInit("debug","","");

    auto& engine = sophon_stream::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json ElementsConfigure;

    ElementsConfigure.push_back(makeDecoderElementConfig(DECODE_ID,"decoder_element","sophgo",0,1,0,false,1, "../lib/libmultiMediaApi.so"));
    ElementsConfigure.push_back(makeElementConfig(REPORT_ID,"report_element","host",0,1,0,false,1, {}));
    nlohmann::json yolov3HeadJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","cocoDetect","Yolov5",
    { "../models/yolov5.bmodel" },
    1, { "000_net" }, { 1 }, {{3, 480, 800}},  {"082_convolutional",
                                "094_convolutional",
                                "106_convolutional"
                               }, { 3}, {{18, 15, 25},{18, 30, 50},{18,60,100}
    },
    { 0.3,0.4 },coco_classnames.size(),coco_classnames);
    ElementsConfigure.push_back(makeElementConfig(YOLO_ID, "action_element", "sophgo", 0, 1, 200, false, 1, {yolov3HeadJson}));
    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    ElementsConfigure.push_back(makeElementConfig(ENCODE_ID,"action_element","host",0,1,200,true,1, {encodeJson}));

    graphConfigure["elements"] = ElementsConfigure;

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,YOLO_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(YOLO_ID,0,ENCODE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID,0,REPORT_ID,0));

    std::mutex mtx;
    std::condition_variable cv;

    engine.addGraph(graphConfigure.dump());

    engine.setDataHandler(1,REPORT_ID,0,[&](std::shared_ptr<void> data) {

        IVS_DEBUG("data output 111111111111111");
        auto objectMetadata = std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
        if(objectMetadata==nullptr) return;
        if(objectMetadata->mFrame->mEndOfStream)
        {
          cv.notify_one();
          return;
        }

        int width = objectMetadata->mFrame->mWidth;
        int height = objectMetadata->mFrame->mHeight;
        // 转格式
        sophon_stream::common::FormatType format_type_stream = objectMetadata->mFrame->mFormatType;
        sophon_stream::common::DataType data_type_stream = objectMetadata->mFrame->mDataType;
        bm_image_format_ext format_type_bmcv = sophon_stream::common::format_stream2bmcv(format_type_stream);
        bm_image_data_format_ext data_type_bmcv = sophon_stream::common::data_stream2bmcv(data_type_stream);
        // 转成bm_image
        bm_image image;
        bm_image_create(objectMetadata->mFrame->mHandle, height, width, format_type_bmcv, 
        data_type_bmcv, &image);
        bm_image_attach(image, objectMetadata->mFrame->mData.get());

        bm_image imageStorage;
        bm_image_create(objectMetadata->mFrame->mHandle, height, width, FORMAT_YUV420P, image.data_type, &imageStorage);
        bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image, &imageStorage);
        bm_image_destroy(image);
      
      for (auto subObj : objectMetadata->mSubObjectMetadatas) {
        
#if DEBUG
        cout << "  class id=" << bbox.class_id << ", score = " << bbox.score << " (x=" << bbox.x << ",y=" << bbox.y << ",w=" << bbox.width << ",h=" << bbox.height << ")" << endl;
#endif
        // draw image
        draw_bmcv(objectMetadata->mFrame->mHandle, subObj->mDetectedObjectMetadata->mClassify, coco_classnames,
        subObj->mDetectedObjectMetadata->mScores[0], subObj->mDetectedObjectMetadata->mBox.mX,
          subObj->mDetectedObjectMetadata->mBox.mY, subObj->mDetectedObjectMetadata->mBox.mWidth,
          subObj->mDetectedObjectMetadata->mBox.mHeight, imageStorage,true);
      }
#if 1
      IVS_DEBUG("data output 666");
        // save image
        void* jpeg_data = NULL;
        size_t out_size = 0;
        int ret = bmcv_image_jpeg_enc(objectMetadata->mFrame->mHandle, 1, &imageStorage, &jpeg_data, &out_size);
        if (ret == BM_SUCCESS) {
          std::string img_file = "a.jpg";
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
    decodeConfigure["url"] = "../test/test_car_person_1080P.mp4";
    //decodeConfigure["url"] = "../test/13.mp4";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;
        
    auto channelTask = std::make_shared<sophon_stream::element::ChannelTask>();
    channelTask->request.operation = sophon_stream::element::ChannelOperateRequest::ChannelOperate::START;
    channelTask->request.json = decodeConfigure.dump();
    sophon_stream::common::ErrorCode errorCode = engine.sendData(1,
                                DECODE_ID,
                                0,
                                std::static_pointer_cast<void>(channelTask),
                                std::chrono::milliseconds(200));
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    usleep(1000000);
}
