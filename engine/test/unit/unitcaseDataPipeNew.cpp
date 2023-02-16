    
/*
 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by xing zilong , 20-03-02

   1. Init.
*/

/**
@file unitcaseDataPipeNew.cpp
@brief 主程序
@details
    DataPipe单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/DataPipeNew.h"
#include <thread>

using namespace lynxi::ivs::common;
using namespace lynxi::ivs::framework;

/**
@brief DataPipe单元测试函数入口

@param [in] TestDataPipe       测试用例命名
@param [in] DefaultConstructor 测试命名
@return void 无返回值


@UnitCase_ID
DataPipe_UT_0002

@UnitCase_Name
unitcaseDataPipeNew

@UnitCase_Description
实例化DataPipe，验证DataPipe的数据队列长度是否为零，验证DataPipe的最大长度是否为64，验证DataPipe的数据队列是否为空，删除DataPipe数据队列中第一个元素，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestDataPipe, DefaultConstructor

@UnitCase_ExpectedResult
DataPipe能正常实例化，测试程序能正常退出。

*/

TEST(TestDataPipe, DefaultConstructor) {

    //create a datapipe with string data
    DataPipe dataPipe;
    ASSERT_EQ(0, dataPipe.getSize());
    ASSERT_EQ(64, dataPipe.getCapacity());
    ASSERT_EQ(nullptr, dataPipe.getData());

    dataPipe.popData();

}

/**
@brief DataPipe单元测试函数入口


@UnitCase_ID
DataPipe_UT_0003

@UnitCase_Name
unitcaseDataPipeNew

@UnitCase_Description
实例化DataPipe，在DataPipe数据队列末尾添加一个新元素‘a’，验证DataPipe的长度是否为1，验证DataPipe的数据队列添加元素是否成功，验证DataPipe数据队列添加元素是否正确。
通过循环依次添加‘0’到‘62’共计63个元素到DataPipe数据队列末尾，并且每添加一个元素后，验证数据队列的长度、返回标志和添加元素是否正常。
在DataPipe数据队列末尾添加一个新元素‘b’,验证数据队列的长度、返回标志和添加元素是否正常。程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestDataPipe, DefaultConstructor

@UnitCase_ExpectedResult
DataPipe能正常实例化和添加数据，测试程序能正常退出。
*/

//测试是否有数据
TEST(TestDataPipe, PushDataAndGetData) {
    DataPipe dataPipe;
    std::shared_ptr<std::string> spA = std::make_shared<std::string>("a");

    auto ret = dataPipe.pushData(spA,std::chrono::seconds(4));
    ASSERT_EQ(1, dataPipe.getSize());
    ASSERT_EQ(ErrorCode::SUCCESS, ret);
    ASSERT_EQ("a", *(std::string*)dataPipe.getData().get());
    for(int i=0;i<63;i++){

        std::shared_ptr<std::string> spString = std::make_shared<std::string>(std::to_string(i));
        ret = dataPipe.pushData(spString,std::chrono::seconds(4));
        ASSERT_EQ(i+2, dataPipe.getSize());
        ASSERT_EQ(ErrorCode::SUCCESS, ret);
        ASSERT_EQ("a", *(std::string*)dataPipe.getData().get());
    }
    std::shared_ptr<std::string> spB = std::make_shared<std::string>("b");

    ret = dataPipe.pushData(spB, std::chrono::milliseconds(200));
    ASSERT_EQ(64, dataPipe.getSize());
    ASSERT_EQ(ErrorCode::TIMEOUT, ret);
    ASSERT_EQ("a", *(std::string*)dataPipe.getData().get());

}

/**
@brief DataPipe单元测试函数入口


@UnitCase_ID
DataPipe_UT_0004

@UnitCase_Name
unitcaseDataPipeNew

@UnitCase_Description
实例化DataPipe，在DataPipe数据队列末尾添加一个新元素‘a’，验证DataPipe的长度是否为1，验证DataPipe的数据队列添加元素是否成功，验证DataPipe数据队列添加元素是否正确。
删除数据队列第一个元素，验证数据队列的长度、最大长度和数据队列是否为空。
通过循环依次添加‘0’到‘62’共计63个元素到DataPipe数据队列末尾，并且每添加一个元素后，验证数据队列的长度、返回标志和添加元素是否正常，然后删除数据队列第一个元素。
在DataPipe数据队列末尾添加一个新元素‘b’,验证数据队列的长度、最大长度、返回标志和添加元素是否正常。程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestDataPipe, DefaultConstructor

@UnitCase_ExpectedResult
DataPipe能正常实例化和添加数据，测试程序能正常退出。
*/

TEST(TestDataPipe, PopData) {
    DataPipe dataPipe;
    std::shared_ptr<std::string> spA = std::make_shared<std::string>("a");

    auto ret = dataPipe.pushData(spA,std::chrono::seconds(4));
    ASSERT_EQ(1, dataPipe.getSize());
    ASSERT_EQ(ErrorCode::SUCCESS, ret);
    ASSERT_EQ("a", *(std::string*)dataPipe.getData().get());
    dataPipe.popData();

    ASSERT_EQ(0, dataPipe.getSize());
    ASSERT_EQ(64, dataPipe.getCapacity());
    ASSERT_EQ(nullptr, dataPipe.getData());
    
    for(int i=0;i<63;i++){

        std::shared_ptr<std::string> spString = std::make_shared<std::string>(std::to_string(i));
        ret = dataPipe.pushData(spString,std::chrono::seconds(4));
        ASSERT_EQ(1, dataPipe.getSize());
        ASSERT_EQ(ErrorCode::SUCCESS, ret);
        ASSERT_EQ(std::to_string(i), *(std::string*)dataPipe.getData().get());
        dataPipe.popData();
    }
    std::shared_ptr<std::string> spB = std::make_shared<std::string>("b");

    ret = dataPipe.pushData(spB, std::chrono::milliseconds(200));
    ASSERT_EQ(64, dataPipe.getCapacity());
    ASSERT_EQ(1, dataPipe.getSize());
    ASSERT_EQ(ErrorCode::TIMEOUT, ret);
    ASSERT_EQ("b", *(std::string*)dataPipe.getData().get());
}
