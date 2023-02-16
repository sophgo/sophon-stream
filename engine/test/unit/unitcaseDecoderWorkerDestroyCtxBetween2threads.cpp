/*
 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by zilong xing <zilong.xing@lynxi.com>, 20-03-16

 NOTE: Simple unit test for data pipe

              constructor->
              start->
              stop->


 Change History:
   1. Init.
*/

#include "gtest/gtest.h"
#include "framework/WorkerFactory.h"
#include "stream/MultiMediaApiFactory.h"
#include "worker/DecoderWorker.h"
#include "common/Logger.h"

#include <opencv2/opencv.hpp>
#include <thread>
#include <unistd.h>
#include <map>

using namespace lynxi::ivs::common;
using namespace lynxi::ivs::framework;

class OutputWorker:public Worker {
  public:
    OutputWorker() {}
    ErrorCode initInternal(const std::string& json) override {
        return ErrorCode::SUCCESS;
    }
    void uninitInternal() override {}
    void onStart()override {
        IVS_INFO("OutputWorker start...");
    }
    void onStop()override {
        IVS_INFO("OutputWorker stop...");
    }
    ErrorCode doWork() override  {
        auto data = getData(0);
        mDatas.push_back(data);
        popData(0);
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
  private:
    std::vector<std::shared_ptr<void>> mDatas;
};
REGISTER_WORKER("output_worker", OutputWorker)

//#define SHOW_VIDEO
using namespace lynxi::ivs::multimedia;
TEST(TestDecoderWorker, DecoderWorker) {
    auto& workerFactory = SingletonWorkerFactory::getInstance();
    auto decoderWorker = std::make_shared<lynxi::ivs::worker::DecoderWorker>();
    auto outputWorker = workerFactory.make("output_worker");

    std::mutex mtx;
    std::condition_variable cv;

    Worker::connect(*std::static_pointer_cast<lynxi::ivs::framework::Worker>(decoderWorker), 0, *outputWorker, 0);
    outputWorker->setDataHandler(0,[&](std::shared_ptr<void> p) {
        auto metaData = std::static_pointer_cast<lynxi::ivs::common::ObjectMetadata>(p);
#ifdef SHOW_VIDEO
        cv::cuda::GpuMat gpuMat(metaData->mFrame->mHeight, metaData->mFrame->mWidth, CV_8UC3, metaData->mFrame->mData.get());
        cv::Mat cpuMat;
        gpuMat.download(cpuMat);
        static int a = 0;
        std::string strPath = "/home/xzl/delete/decode"+std::to_string(a)+".jpg";
        a++;
        cv::imshow("123",cpuMat);
        cv::waitKey(1);

//        cv::imshow(std::to_string(metaData->getChannelId()),cpumat);
//        cv::waitKey(1);
#endif
        if(metaData->mFrame->mEndOfStream)
        {
            sleep(5);
            cv.notify_one();
        }
    });

    nlohmann::json workerConfigure;
    workerConfigure["id"] = 0;
    workerConfigure["side"] = "nvidia";
    workerConfigure["device_id"] = 0;
    workerConfigure["milliseconds_timeout"] = 200;
    workerConfigure["repeated_timeout"] = true;
    nlohmann::json moduleConfigure;
    moduleConfigure["shared_object"] = "../lib/libmultiMediaApi.so";
    workerConfigure["configure"] = moduleConfigure;

    lynxi::ivs::common::ErrorCode errorCode = decoderWorker->init(workerConfigure.dump());
    ASSERT_EQ(lynxi::ivs::common::ErrorCode::SUCCESS, errorCode);
    decoderWorker->start();
    outputWorker->start();
    for(int i=0; i<1; i++) {
        std::shared_ptr<lynxi::ivs::worker::ChannelTask> data = std::make_shared<lynxi::ivs::worker::ChannelTask>();
        data->request.channelId = i;
        data->request.operation = lynxi::ivs::worker::ChannelOperateRequest::ChannelOperate::START;
        nlohmann::json decodeConfigure;
        //decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.24:554/Streaming/Channels/101?transportmode=unicast";
        decodeConfigure["url"] = "../test/out-720p-2s.mp4";
        //decodeConfigure["url"] = "../test/3.avi";
        //decodeConfigure["url"] = "../test/13.mp4";
        decodeConfigure["resize_rate"] = 3.0f;
        decodeConfigure["timeout"] = 0;
        decodeConfigure["multimedia_name"] = "decode_picture";
        decodeConfigure["source_type"] = 2;
        decodeConfigure["reopen_times"] = -1;

        data->request.json = decodeConfigure.dump();
        std::static_pointer_cast<lynxi::ivs::worker::DecoderWorker>(decoderWorker)->pushData(0, data, std::chrono::seconds(4));
    }
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    //析构调用了 这里不调
    decoderWorker->stop();
    outputWorker->stop();
}
