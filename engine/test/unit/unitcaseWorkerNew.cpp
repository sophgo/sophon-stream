/*
 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by zilong xing <zilong.xing@lynxi.com>, 18-9-19

 NOTE: Simple unit test for data pipe

              constructor->
              start->
              stop->


 Change History:
   1. Init.
*/

/**
@file unitcaseWorkerNew.cpp
@brief 主程序
@details
    WorkerNew单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/WorkerNew.h"
#include "framework/WorkerFactory.h"
#include "common/Logger.h"
#include <thread>
#include <unistd.h>
#include <map>

using namespace lynxi::ivs::common;
using namespace lynxi::ivs::framework;

/**
@brief WorkerSource类
@details
    继承于Work类，重写了各个函数
*/

class WorkerSource : public Worker {
  public:
    WorkerSource() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        IVS_INFO("WorkerSource start...");
    }
    void onStop()override{
        IVS_INFO("WorkerSource stop...");
    }
    ErrorCode doWork() override  {

        auto data = getData(0);
        popData(0);
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerSource", WorkerSource)

/**
@brief WorkerMid类
@details
    继承于Work类，重写了各个函数
*/

class WorkerMid : public Worker {
  public:
    WorkerMid() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        IVS_INFO("WorkerMid start...");
    }
    void onStop()override{
        IVS_INFO("WorkerMid stop...");
    }
    ErrorCode doWork() override {
        auto data = getData(0);
        popData(0);
        sendData(0, data, std::chrono::seconds(4));
        //getData(0);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerMid", WorkerMid)

/**
@brief WorkerSink类
@details
    继承于Work类，重写了各个函数
*/

class WorkerSink: public Worker{
    public:
        WorkerSink(){}
        ErrorCode initInternal(const std::string& json) override{}
        void uninitInternal() override{}
        void onStart()override{
            IVS_INFO("WorkerSink start...");
        }
        void onStop()override{
            IVS_INFO("WorkerSink stop...");
        }

        ErrorCode doWork() override{
            if(getDataCount(0)==0) return ErrorCode::SUCCESS;
            auto data = getData(0);
            popData(0);
            sendData(0, data, std::chrono::seconds(4));
            return ErrorCode::SUCCESS;
        }
};
REGISTER_WORKER("WorkerSink", WorkerSink)

/**
@brief WorkerNew单元测试函数入口

@param [in] TestWorker    测试用例命名
@param [in] OnepPipeLine  测试命名
@return void 无返回值


@UnitCase_ID
Worker_UT_0014

@UnitCase_Name
unitcaseWorkerNew

@UnitCase_Description
实例化sourceWorker，实例化midWorker，实例化sinkWorker，按顺序连接这三个Worker，设置sinkWorker端口0的回调函数，启动sourceWorker、midWorker和sinkWorker，
sourceWorker发送数据，sinkWorker接收处理数据，并确认接收数据是否正确，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestWorker, OnepPipeLine

@UnitCase_ExpectedResult
sourceWorker、midWorker和sinkWorker能正常实例化和启动，并按照预定顺序先后连接，sourceWorker正常发送数据，sinkWorker正常接收处理数据，输入输出数据保持一致

*/

TEST(TestWorker, OnepPipeLine) {
    auto& workerFactory = SingletonWorkerFactory::getInstance();
    auto workerSource = workerFactory.make("WorkerSource");
    auto workerMid = workerFactory.make("WorkerMid");
    auto workerSink = workerFactory.make("WorkerSink");

    std::string strResult;
    std::mutex mtx;
    std::condition_variable cv;

    Worker::connect(*workerSource, 0, *workerMid, 0);
    Worker::connect(*workerMid, 0, *workerSink, 0);
    workerSink->setDataHandler(0,[&](std::shared_ptr<void> p){
            int result = *std::static_pointer_cast<int>(p);
            strResult += std::to_string(result);
            if(result==9){
                cv.notify_one();
            }
            });
    workerSource->start();
    workerMid->start();
    workerSink->start();
    for(int i=0;i<10;i++){
        std::shared_ptr<int> data = std::make_shared<int>(i);
        std::static_pointer_cast<WorkerSource>(workerSource)->pushData(0, data, std::chrono::seconds(4));
    }
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    workerSink->stop();
    workerMid->stop();
    workerSource->stop();
    EXPECT_EQ(strResult, "0123456789");
}
