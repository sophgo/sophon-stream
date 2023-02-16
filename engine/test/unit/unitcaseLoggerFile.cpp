/*
 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by zilong xing <zilong.xing@lynxi.com>, 18-9-28

 NOTE: test singleton spdlog
*/

/**
@file unitcaseLoggerFile.cpp
@brief 主程序
@details
    LoggerFile单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "common/Logger.h"
#include <thread>
#include <unistd.h>

/**
@brief LoggerFile单元测试函数入口

@param [in] LogFile  测试用例命名
@param [in] Log      测试命名
@return void 无返回值


@UnitCase_ID
12345

@UnitCase_Name
unitcaseLoggerFile

@UnitCase_Description
控制台输出ERROR信息，控制台输出WARN信息，控制台输出CRITICAL信息，控制台输出多个不同INFO信息，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
LogFile, Log

@UnitCase_ExpectedResult
控制台能正常输出各种类型的信息

*/

//测试在文件打印
TEST(LogFile, Log) {
    IVS_ERROR("Some error message");
    IVS_WARN("Easy padding in numbers like {:08d}", 12);
    IVS_CRITICAL("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    IVS_INFO("Support for floats {:03.2f}", 1.23456);
    IVS_INFO("Positional args are {1} {0}..", "too", "supported");
    IVS_INFO("{:<30}", "left aligned");
}
