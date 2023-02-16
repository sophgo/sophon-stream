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
@file unitcaseWorkerNew-Diamond.cpp
@brief 主程序
@details
    WorkerNew-Diamond单元测试

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

#include<sys/prctl.h>
//测试开始和结束，m_ids存储了每个线程的id
using namespace lynxi::ivs::common;
using namespace lynxi::ivs::framework;

/**
@brief Packet结构体
@details
    用于worker类方法计算
*/

struct Packet{
    std::vector<int> a;//a为b, c, d的集合
    int b;
    int c;
    int d;
    int e;//e为b, c, d之和
};

/**
@brief WorkerA类
@details
    继承于Work类，重写了各个函数
*/

class WorkerA:public Worker{
  public:
    WorkerA() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        std::string threadname = "A";
        prctl(PR_SET_NAME, threadname.c_str());
        IVS_INFO("WorkerA start...");
    }
    void onStop()override{
        IVS_INFO("WorkerA stop...");
    }
    ErrorCode doWork() override  {
        auto data = getData(0);
        popData(0);
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        ret = sendData(1, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        ret = sendData(2, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerA", WorkerA)

/**
@brief WorkerB类
@details
    继承于Work类，重写了各个函数
*/

class WorkerB:public Worker{

  public:
    WorkerB() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        std::string threadname = "B";
        prctl(PR_SET_NAME, threadname.c_str());
        IVS_INFO("WorkerA start...");
    }
    void onStop()override{
        IVS_INFO("WorkerA stop...");
    }
    ErrorCode doWork() override  {
        auto data = getData(0);
        popData(0);
        std::static_pointer_cast<Packet>(data)->b = std::static_pointer_cast<Packet>(data)->a[0];
        if(data==nullptr) IVS_ERROR("B error");
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerB", WorkerB)

/**
@brief WorkerC类
@details
    继承于Work类，重写了各个函数
*/

class WorkerC:public Worker{
  public:
    WorkerC() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        std::string threadname = "C";
        prctl(PR_SET_NAME, threadname.c_str());
        IVS_INFO("WorkerC start...");
    }
    void onStop()override{
        IVS_INFO("WorkerC stop...");
    }
    ErrorCode doWork() override  {
        auto data = getData(0);
        popData(0);
        std::static_pointer_cast<Packet>(data)->c = std::static_pointer_cast<Packet>(data)->a[1];
        if(data==nullptr) IVS_ERROR("C error");
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerC", WorkerC)

/**
@brief WorkerD类
@details
    继承于Work类，重写了各个函数
*/

class WorkerD:public Worker{
  public:
    WorkerD() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{
        std::string threadname = "D";
        prctl(PR_SET_NAME, threadname.c_str());
        IVS_INFO("WorkerD start...");
    }
    void onStop()override{
        IVS_INFO("WorkerD stop...");
    }
    ErrorCode doWork() override  {
        auto data = getData(0);
        popData(0);
        std::static_pointer_cast<Packet>(data)->d = std::static_pointer_cast<Packet>(data)->a[2];
        if(data==nullptr) IVS_ERROR("D error");
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("WorkerD", WorkerD)

/**
@brief WorkerE类
@details
    继承于Work类，重写了各个函数
*/

class WorkerE:public Worker{
  public:
    WorkerE() {}
    ErrorCode initInternal(const std::string& json) override{}
    void uninitInternal() override{}
    void onStart()override{

        std::string threadname = "E";
        prctl(PR_SET_NAME, threadname.c_str());
        IVS_INFO("WorkerE start...");
    }
    void onStop()override{
        IVS_INFO("WorkerE stop...");
    }
    ErrorCode doWork() override  {
        if(getDataCount(0)==0||getDataCount(1)==0||getDataCount(2)==0) return ErrorCode::SUCCESS;
        auto data = getData(0);
        popData(0);
        data = getData(1);
        popData(1);
        data = getData(2);
        popData(2);
        std::static_pointer_cast<Packet>(data)->e += std::static_pointer_cast<Packet>(data)->b;
        std::static_pointer_cast<Packet>(data)->e += std::static_pointer_cast<Packet>(data)->c;
        std::static_pointer_cast<Packet>(data)->e += std::static_pointer_cast<Packet>(data)->d;
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }

};
REGISTER_WORKER("WorkerE", WorkerE)

/**
@brief WorkerNew-Diamond单元测试函数入口

@param [in] TestWorker    测试用例命名
@param [in] DiamondGraph  测试命名
@return void 无返回值


@UnitCase_ID
Worker_UT_0013

@UnitCase_Name
unitcaseWorkerNew-Diamond

@UnitCase_Description
实例化workerA、workerB、workerC、workerD和workerE，将workerA的0、1和2端口分别连接到workerB、workerC和workerD，
将workerB、workerC和workerD的0端口分别连接到workerE的0、1和2端口，设置workerE端口0的回调函数，workerA发送数据，workerE接收处理数据，并验证输入数据和输出数据是否一致

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestWorker, DiamondGraph

@UnitCase_ExpectedResult
workerA、workerB、workerC、workerD和workerE能正常实例化，并按照预定顺序先后连接，workerA正常发送数据，workerE正常接收处理数据，输入输出数据保持一致

*/

TEST(TestWorker, DiamondGraph){
    auto& workerFactory = SingletonWorkerFactory::getInstance();
    auto workerA = workerFactory.make("WorkerA");
    auto workerB = workerFactory.make("WorkerB");
    auto workerC = workerFactory.make("WorkerC");
    auto workerD = workerFactory.make("WorkerD");
    auto workerE = workerFactory.make("WorkerE");

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::vector<int>> input{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    std::vector<Packet> output;
    Worker::connect(*workerA, 0, *workerB, 0);
    Worker::connect(*workerA, 1, *workerC, 0);
    Worker::connect(*workerA, 2, *workerD, 0);
    Worker::connect(*workerB, 0, *workerE, 0);
    Worker::connect(*workerC, 0, *workerE, 1);
    Worker::connect(*workerD, 0, *workerE, 2);
    workerE->setDataHandler(0,[&](std::shared_ptr<void> p){
                IVS_INFO("diamond result");
                output.push_back(*std::static_pointer_cast<Packet>(p));
                if(std::static_pointer_cast<Packet>(p)->e==24){
                    cv.notify_one();
                }
            });
    workerA->start();
    workerB->start();
    workerC->start();
    workerD->start();
    workerE->start();
    for(auto& element:input){
        std::shared_ptr<Packet> sp = std::make_shared<Packet>();
        sp->a = element;
        IVS_INFO("push to a");
        std::static_pointer_cast<WorkerA>(workerA)->pushData(0, sp, std::chrono::seconds(4));
    }
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    workerA->stop();
    workerB->stop();
    workerC->stop();
    workerD->stop();
    workerE->stop();
    ASSERT_EQ(input.size(), output.size());
    for(int i=0;i<input.size();i++){
        ASSERT_EQ(input[i][0]+input[i][1]+input[i][2], output[i].e);
    }
}
