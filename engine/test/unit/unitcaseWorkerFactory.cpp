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
@file unitcaseWorkerFactory.cpp
@brief 主程序
@details
    WorkerFactory单元测试

@author lzw
@date 2020-06-16
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/WorkerFactory.h"

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

/**
@brief WorkerFactory单元测试函数入口

@param [in] TestWorkerFactory 测试用例命名
@param [in] OnepPipeLine      测试命名
@return void 无返回值


@UnitCase_ID
Worker_UT_0015

@UnitCase_Name
unitcaseWorkerFactory

@UnitCase_Description
实例化WorkerFactory，往WorkerMaker中添加成员"WorkerSource",实例化Worker "WorkerSource"，从WorkerMaker中移除成员"WorkerSource"，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestWorkerFactory, OnepPipeLine

@UnitCase_ExpectedResult
WorkerFactory和Worker "WorkerSource"能正常实例化

*/

TEST(TestWorkerFactory, OnepPipeLine) {
    std::string workerName = "WorkerSource";

    auto& workerFactory = SingletonWorkerFactory::getInstance();
    ASSERT_NE(&workerFactory ,nullptr);

    ErrorCode ret = workerFactory.addWorkerMaker(workerName, []() { 
                                             return std::make_shared<WorkerSource>(); 
                                         }); 

    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    auto workerSource = workerFactory.make(workerName);
    ASSERT_NE(workerSource.get() ,nullptr);

    workerFactory.removeWorkerMaker(workerName);
}
