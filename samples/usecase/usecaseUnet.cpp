#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "element/MandatoryLink.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/type_trans.hpp"
#include <opencv2/opencv.hpp>
#include "config.h"
// #include <fstream>

#define DECODE_ID 5000
#define UNET_ID 5001
#define ENCODE_ID 5006
#define REPORT_ID 5555



TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph)
{
    ::logInit("debug", "", "");

    auto & engine = sophon_stream::framework::SingletonEngine::getInstance(); 

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json ElementsConfigure;

    ElementsConfigure.push_back(makeDecoderElementConfig(DECODE_ID, "decoder_element", "sophgo", 0, 1, 0, false, 1, "../lib/libmultiMediaApi.so"));
    ElementsConfigure.push_back(makeElementConfig(REPORT_ID, "report_element", "host", 0, 1, 0, false, 1, {}));
    nlohmann::json unetJson = makeAlgorithmConfig("../lib/libalgorithmApi.so", "carSegment", "Unet",
        {"../models/unetBM1684.bmodel"},
        1, {"input"}, {1}, {{3, 640, 959}}, {"456"}, {1}, 
        {{2, 640, 959}}, {0.5}, 2, {"foreground", "background"});
    
    ElementsConfigure.push_back(makeElementConfig(UNET_ID, "action_element", "sophgo", 0, 1, 200, false, 1, {unetJson}));
    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so", "", "encode_picture", 1);
    ElementsConfigure.push_back(makeElementConfig(ENCODE_ID,"action_element","host",0,1,200,true,1, {encodeJson}));

    graphConfigure["elements"] = ElementsConfigure;

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID, 0, UNET_ID, 0));
    graphConfigure["connections"].push_back(makeConnectConfig(UNET_ID, 0, ENCODE_ID, 0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID, 0, REPORT_ID, 0));

    std::mutex mtx;
    std::condition_variable cv;

    engine.addGraph(graphConfigure.dump());

    engine.setDataHandler(1, REPORT_ID, 0, [&](std::shared_ptr<void> data){
        IVS_DEBUG("data output------------------");
        auto objectMetadata = std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
        if(objectMetadata == nullptr)   return;

        if(objectMetadata->mSegmentedObjectMetadatas[0]->mFrame != nullptr && 
            objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mSpData != nullptr)
        {

            int width = objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mWidth;
            int height = objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mHeight;
            // 转成bm_image
            bm_image image1 = *objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mSpData;
            
            static int idx = 0;
            char szpath[256] = {0}; 
            sprintf(szpath,"UnetResultyyy%d.bmp",idx);
            std::string strPath(szpath);
            bm_image_write_to_bmp(image1, strPath.c_str());
            idx++;

            if(objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mEndOfStream) cv.notify_one();
        }
    });

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    // decodeConfigure["url"] = "../test/car_white.jpg";
    decodeConfigure["url"] = "../test/carvana_video.mp4";
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
}