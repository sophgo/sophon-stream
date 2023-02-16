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
@file unitcasePreModuleWorker.cpp
@brief 主程序
@details
    PreModuleWorker单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/PreModuleWorker.cpp"
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
@brief WorkerAction类
@details
    继承于Work类，重写了各个函数
*/

class WorkerAction : public Worker {
  public:
    WorkerAction() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        IVS_INFO("WorkerAction start...");
    }
    void onStop()override{
        IVS_INFO("WorkerAction stop...");
    }
    ErrorCode doWork() override  {
        auto data = getData(0);
        popData(0);
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerAction", WorkerAction)

/**
@brief PreModuleWorker单元测试函数入口

@param [in] TestPreModuleWorker  测试用例命名
@param [in] PreModuleWorker      测试命名
@return void 无返回值


@UnitCase_ID
PreModuleWorker_UT_0010

@UnitCase_Name
unitcasePreModuleWorker

@UnitCase_Description
实例化sourceWorker，实例化actionWorker，实例化preModuleWorker，按顺序连接这三个Worker，初始化preModuleWorker，启动actionWorker和preModuleWorker，
sourceWorker发送数据，actionWorker接收处理数据，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestPreModuleWorker, PreModuleWorker

@UnitCase_ExpectedResult
PreModuleWorker、actionWorker和sourceWorker能正常实例化和启动，sourceWorker正常发送数据，actionWorker正常接收处理数据，程序能正常退出

*/

TEST(TestPreModuleWorker, PreModuleWorker) {
    auto& workerFactory = SingletonWorkerFactory::getInstance();
    auto workerSource = workerFactory.make("WorkerSource");
    auto workerAction = workerFactory.make("WorkerAction");
    auto preModuleWorker = workerFactory.make("pre_module_worker");
    
    std::mutex mtx;
    std::condition_variable cv;
   
    Worker::connect(*workerSource, 0, *preModuleWorker, 0);
    Worker::connect(*preModuleWorker, 0, *workerAction, 0);
    workerAction->setDataHandler(0,[&](std::shared_ptr<void> p){
            cv.notify_one();
            });

    std::string json = "{\"id\":1000, \"side\":\"cpu\", \"device_id\":0, \"thread_number\":1, \"milliseconds_timeout\":100,\"repeated_timeout\":true, \"configure\":{\"module_count\":2} }";
    ErrorCode ret = preModuleWorker->init(json);
    ASSERT_EQ(ret = ErrorCode::SUCCESS, ret);
    workerAction->start();
    preModuleWorker->start();
    for(int i=0;i<10;i++){
        std::shared_ptr<ObjectMetadata> data = std::make_shared<ObjectMetadata>();
        std::static_pointer_cast<WorkerSource>(workerSource)->pushData(0, data, std::chrono::seconds(4));
    }
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    workerAction->stop();
    preModuleWorker->stop();
    //EXPECT_EQ(strResult, "0123456789");
}
