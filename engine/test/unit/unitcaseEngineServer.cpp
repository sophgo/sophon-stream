/**
@file unitcaseEngineServer.cpp
@brief 主程序
@details
    引擎服务端启动停止测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "common/Logger.h"
#include "grpc/EngineServer.h"
#include <thread>
#include <unistd.h>

/**
@brief 引擎服务端控制单元测试函数入口

@param [in] EngineServer 测试用例命名
@param [in] RunAndStop   测试命名
@return void 无返回值


@UnitCase_ID
EngineServer_UT_0006

@UnitCase_Name
unitcaseEngineServer

@UnitCase_Description
开启引擎服务，五秒后停止引擎服务，端口默认为50051，测试是否运行正常

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
EngineServer, RunAndStop

@UnitCase_ExpectedResult
引擎服务能正常运行和停止
*/

//测试在控制台打印
TEST(EngineServer, RunAndStop) {

    lynxi::ivs::grpc::EngineServer engineServer;

    std::shared_ptr<std::thread> spThread = std::make_shared<std::thread>([&]() {
        sleep(5);
        engineServer.stop();
    });

    engineServer.run("0.0.0.0:50051",1024*1024*100,1024*1024*100);

    spThread->join();
}
