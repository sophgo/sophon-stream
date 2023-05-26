#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "element/multimedia/src/decode/DecoderElement.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"
#include "common/type_trans.hpp"
#include <opencv2/opencv.hpp>
#include "config.h"
#include <fstream>

#define DECODE_ID 5000
#define PRE_ID 5001
#define UNET_ID 5002
#define POST_ID 5003

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

    ::logInit("debug", "", "");

    auto & engine = sophon_stream::framework::SingletonEngine::getInstance(); 

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json ElementsConfigure;

    std::ifstream istream;
    nlohmann::json decoder, pre, action, post, encoder, reporter;

    istream.open("../usecase/json/unet/Decoder.json");
    assert(istream.is_open());
    istream >> decoder;
    decoder.at("id") = DECODE_ID;
    ElementsConfigure.push_back(decoder);
    istream.close();
    std::cout << decoder << std::endl;

    istream.open("../usecase/json/unet/Pre.json");
    assert(istream.is_open());
    istream >> pre;
    pre.at("id") = PRE_ID;
    ElementsConfigure.push_back(pre);
    istream.close();
    std::cout << pre << std::endl;

    istream.open("../usecase/json/unet/Action.json");
    assert(istream.is_open());
    istream >> action;
    action.at("id") = UNET_ID;
    ElementsConfigure.push_back(action);
    istream.close();

    istream.open("../usecase/json/unet/Post.json");
    assert(istream.is_open());
    istream >> post;
    post.at("id") = POST_ID;
    ElementsConfigure.push_back(post);
    istream.close();

    graphConfigure["elements"] = ElementsConfigure;

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID, 0, PRE_ID, 0));
    graphConfigure["connections"].push_back(makeConnectConfig(PRE_ID, 0, UNET_ID, 0));
    graphConfigure["connections"].push_back(makeConnectConfig(UNET_ID, 0, POST_ID, 0));

    std::mutex mtx;
    std::condition_variable cv;

    engine.addGraph(graphConfigure.dump());

    engine.setDataHandler(1, POST_ID, 0, [&](std::shared_ptr<void> data){
        IVS_DEBUG("data output------------------");
        auto objectMetadata = std::static_pointer_cast<sophon_stream::common::ObjectMetadata>(data);
        if(objectMetadata == nullptr || objectMetadata->mSegmentedObjectMetadatas.size() == 0)   return;
        if(objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mEndOfStream) cv.notify_one();
        if(objectMetadata->mSegmentedObjectMetadatas[0]->mFrame != nullptr && 
            objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mSpData != nullptr)
        {

            int width = objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mWidth;
            int height = objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mHeight;
            // 转成bm_image
            bm_image image1 = *objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mSpData;
            
            static int idx = 0;
            char szpath[256] = {0}; 
            sprintf(szpath,"./results/UnetResult%d.bmp",idx);
            std::string strPath(szpath);
            bm_image_write_to_bmp(image1, strPath.c_str());
            idx++;
            // bm_image_destroy(image1);
            // if(objectMetadata->mSegmentedObjectMetadatas[0]->mFrame->mEndOfStream) cv.notify_one();
        }
    });

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    // decodeConfigure["url"] = "../test/car_white.jpg";
    decodeConfigure["url"] = "../carvana_video.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;

    auto channelTask = std::make_shared<sophon_stream::multimedia::ChannelTask>();
    channelTask->request.operation = sophon_stream::multimedia::ChannelOperateRequest::ChannelOperate::START;
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

    engine.stop(1);
}