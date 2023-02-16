/*
 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by yang jun <jun.yang@lynxi.com>, 20-3-17
*/

/**
@file unitcaseActionWorker.cpp
@brief 主程序
@details
    ActionWorker单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include <condition_variable>
#include <mutex>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "worker/ActionWorker.h"
#include "common/Logger.h"

/**
@brief ActionWorker单元测试函数入口

@param [in] TestActionWorker 测试用例命名
@param [in] InitAndDoWork    测试命名
@return void 无返回值


@UnitCase_ID
ActionWorker_UT_0001

@UnitCase_Name
unitcaseActionWorker

@UnitCase_Description
实例化ActionWorker，以face_attributes为例来初始化ActionWorker并验证是否初始化成功，启动ActionWorker并验证是否启动成功，设置ActionWorker输出端口和回调函数，
输出端口默认为0，该回调函数为lamda匿名函数，ActionWorker发送数据，回调函数接受到数据并进行验证，停止ActionWorker，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestActionWorker, InitAndDoWork

@UnitCase_ExpectedResult
ActionWorker能正常实例化，初始化，启动，发送接受数据和停止，测试程序能正常退出。
*/

TEST(TestActionWorker, InitAndDoWork) {
    auto actionWorker = std::make_shared<lynxi::ivs::worker::ActionWorker>();

    nlohmann::json workerConfigure;
    workerConfigure["id"] = 0;
    workerConfigure["side"] = "nvidia";
    workerConfigure["device_id"] = 0;
    workerConfigure["milliseconds_timeout"] = 200;
    workerConfigure["repeated_timeout"] = true;

    nlohmann::json actionWorkerConfigure;
    actionWorkerConfigure["batch"] = 4;

    nlohmann::json modelConfigure;
    modelConfigure["shared_object"] = "/home/yangjun/ivs/ivs-algorithm-api-new/build/libalgorithmApi.so";
    modelConfigure["name"] = "age";
    modelConfigure["algorithm_name"] = "face_attributes";
    modelConfigure["model_path"] = { "/home/yangjun/ivs/ivs-algorithm-api-new/model/trtAgeGender.txt" };
    modelConfigure["max_batchsize"] = 35;
    modelConfigure["input_node_name"] = { "data" };
    modelConfigure["num_inputs"] = { 1 };
    modelConfigure["input_shape"] = { { 3, 112, 112 } };
    modelConfigure["output_node_name"] = { "fc1" };
    modelConfigure["num_outputs"] = { 1 };
    modelConfigure["output_shape"] = { { 202, 1, 1 } };
    modelConfigure["threthold"] = { 0.8 };

    actionWorkerConfigure["models"] = { modelConfigure };
    workerConfigure["configure"] = actionWorkerConfigure;

    lynxi::ivs::common::ErrorCode errorCode = lynxi::ivs::common::ErrorCode::SUCCESS; 

    errorCode = actionWorker->init(workerConfigure.dump());
    ASSERT_EQ(lynxi::ivs::common::ErrorCode::SUCCESS, errorCode);

    errorCode = actionWorker->start();
    ASSERT_EQ(lynxi::ivs::common::ErrorCode::SUCCESS, errorCode);

    std::mutex mutex;
    std::condition_variable cv;
    actionWorker->setDataHandler(0, 
                                 [&cv](std::shared_ptr<void> data) {
                                     if (!data) {
                                         return;
                                     }

                                     auto objectMetadata = std::static_pointer_cast<lynxi::ivs::common::ObjectMetadata>(data);
                                     EXPECT_NE(lynxi::ivs::common::ErrorCode::SUCCESS, objectMetadata->mErrorCode);

                                     cv.notify_one();
                                 });

    auto objectMetadata = std::make_shared<lynxi::ivs::common::ObjectMetadata>();
    actionWorker->pushData(0, 
                           std::static_pointer_cast<void>(objectMetadata), 
                           std::chrono::milliseconds(200));

    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock);

    errorCode = actionWorker->stop();
    ASSERT_EQ(lynxi::ivs::common::ErrorCode::SUCCESS, errorCode);
}
