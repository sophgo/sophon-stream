/**
@file unitcaseMultiMediaApi.cpp
@brief 主程序
@details
    MultiMediaApi单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include <dlfcn.h>
#include "common/Logger.h"
#include "stream/MultiMediaApi.h"
#include "stream/MultiMediaApiFactory.h"
#include "common/ObjectMetadata.h"
#include <opencv2/opencv.hpp>
#define SHOW_VIDEO

/**
@brief MultiMediaApi单元测试函数入口

@param [in] TestDecoderApi  测试用例命名
@param [in] DecoderApi      测试命名
@return void 无返回值


@UnitCase_ID
MultiMediaApi_UT_0008

@UnitCase_Name
unitcaseMultiMediaApi

@UnitCase_Description
实例化multimediaApi，初始化multimedia，循环调用multimedia的process方法，实时处理返回数据，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestDecoderApi, DecoderApi

@UnitCase_ExpectedResult
multimedia能正常实例化、初始化和运行process方法，能实时处理返回数据，处理完所有数据程序能正常退出

*/

TEST(TestDecoderApi, DecoderApi) {
    void* sharedObjectHandle = dlopen("../lib/libmultiMediaApi.so", RTLD_NOW | RTLD_GLOBAL);
    if (NULL == sharedObjectHandle) {
        IVS_ERROR("Load dynamic shared object file fail, error info:{0}",dlerror());
        return;
    }

    std::shared_ptr<void> spSharedObjectHandle(sharedObjectHandle,
    [](void* sharedObjectHandle) {
        dlclose(sharedObjectHandle);
    });



    auto& multimediaApiFactory = lynxi::ivs::multimedia::SingletonMultiMediaApiFactory::getInstance();
    std::shared_ptr<lynxi::ivs::multimedia::MultiMediaApi> spDecoder = multimediaApiFactory.make();
    if (!spDecoder) {
        return;
    }

    nlohmann::json decodeConfigure;
    // decodeConfigure["url"] = "../test/13.mp4";
    decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.12:554/Streaming/Channels/101?transportmode=unicast";
    //decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.24:554/Streaming/Channels/101?transportmode=unicast";
    //decodeConfigure["url"] = "rtsp://lynxi:Lxcam2020@192.168.54.135:554/Streaming/Channels/1601?transportmode=unicast";  // wh
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";

    lynxi::ivs::common::ErrorCode ret = spDecoder->init("nvidia",
                                   0,
                                   decodeConfigure.dump());
    if (lynxi::ivs::common::ErrorCode::SUCCESS != ret) {
        IVS_ERROR("decoder init failed! {0}",(int)ret);
        return;
    }

    while(ret!=lynxi::ivs::common::ErrorCode::STREAM_END) {
        std::shared_ptr<lynxi::ivs::common::ObjectMetadata> objectMetadata;
        ret = spDecoder->process(objectMetadata);
        if(objectMetadata->mFrame==nullptr||ret!=lynxi::ivs::common::ErrorCode::SUCCESS) {
            IVS_ERROR("decode process failed:{0}", (int)ret);
            continue;
        }
#ifdef SHOW_VIDEO
        cv::cuda::GpuMat gpuMat(objectMetadata->mFrame->mHeight, objectMetadata->mFrame->mWidth, CV_8UC3, objectMetadata->mFrame->mData.get());
        cv::Mat cpuMat;
        gpuMat.download(cpuMat);
        cv::imshow("123",cpuMat);
        cv::waitKey(1);
        static int count = 0;
        count++;
        if(count==100)break;

//        cv::imshow(std::to_string(objectMetadata->getChannelId()),cpumat);
//        cv::waitKey(1);
#endif
        
    }

//    lynxi::ivs::stream::DecoderApi decodeApi;
//    nlohmann::json config;
//    config["url"] = "../test/13.mp4";
//    config["resize_rate"] = 3.0f;
//    config["timeout"] = 0;
//    config["source_type"] = 0;
//    lynxi::ivs::common::ErrorCode ret = decodeApi.init("host",0,config.dump());
//    ASSERT_EQ(ret,lynxi::ivs::common::ErrorCode::SUCCESS);
//    while(ret!=lynxi::ivs::common::ErrorCode::STREAM_END) {
//        std::shared_ptr<lynxi::ivs::common::Frame> frame;
//        ret = decodeApi.getFrame(frame);
//        if(frame==nullptr) continue;
//    }
//    decodeApi.uninit();
}

/**
@brief MultiMediaApi单元测试函数入口

@param [in] TestDecoderApi  测试用例命名
@param [in] WrongUrl      测试命名
@return void 无返回值


@UnitCase_ID
MultiMediaApi_UT_0009

@UnitCase_Name
unitcaseMultiMediaApi

@UnitCase_Description
实例化multimediaApi，以错误参数初始化multimedia，验证是否初始化失败，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestDecoderApi, WrongUrl

@UnitCase_ExpectedResult
multimedia能正常实例化，输入错误参数进行初始化会失败，程序能正常退出

*/

TEST(TestDecoderApi, WrongUrl) {
    auto& multimediaApiFactory = lynxi::ivs::multimedia::SingletonMultiMediaApiFactory::getInstance();
    std::shared_ptr<lynxi::ivs::multimedia::MultiMediaApi> spDecoder = multimediaApiFactory.make();
    if (!spDecoder) {
        return;
    }

    nlohmann::json decodeConfigure;
    // decodeConfigure["url"] = "../test/13.mp4";
    decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17sdf.24:554/Streaming/Channels/101?transportmode=unicast";
    //decodeConfigure["url"] = "rtsp://lynxi:Lxcam2020@192.168.54.135:554/Streaming/Channels/1601?transportmode=unicast";  // wh
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";

    lynxi::ivs::common::ErrorCode ret = spDecoder->init("nvidia",
                                   0,
                                   decodeConfigure.dump());
    ASSERT_EQ(ret,lynxi::ivs::common::ErrorCode::ERR_FFMPEG_INPUT_CTX_OPEN);

}
