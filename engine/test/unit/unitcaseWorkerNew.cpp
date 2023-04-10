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
@file unitcaseElementNew.cpp
@brief 主程序
@details
    ElementNew单元测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/ElementNew.h"
#include "framework/ElementFactory.h"
#include "common/Logger.h"
#include <thread>
#include <unistd.h>
#include <map>

using namespace sophon_stream::common;
using namespace sophon_stream::framework;

/**
@brief ElementSource类
@details
    继承于Work类，重写了各个函数
*/

class ElementSource : public Element {
  public:
    ElementSource() {}
    ErrorCode initInternal(const std::string& json) override{return sophon_stream::common::ErrorCode::SUCCESS;}
    void uninitInternal() override{}
    void onStart()override{
        IVS_INFO("ElementSource start...");
    }
    void onStop()override{
        IVS_INFO("ElementSource stop...");
    }
    ErrorCode doWork() override  {

        auto data = getData(0);
        popData(0);
        ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
        if(ret!=ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("ElementSource", ElementSource)

/**
@brief ElementMid类
@details
    继承于Work类，重写了各个函数
*/

class ElementMid : public Element {
  public:
    ElementMid() {}
    ErrorCode initInternal(const std::string& json) override{return sophon_stream::common::ErrorCode::SUCCESS;}
    void uninitInternal() override{}
    void onStart()override{
        IVS_INFO("ElementMid start...");
    }
    void onStop()override{
        IVS_INFO("ElementMid stop...");
    }
    ErrorCode doWork() override {
        auto data = getData(0);
        popData(0);
        sendData(0, data, std::chrono::seconds(4));
        //getData(0);
        return ErrorCode::SUCCESS;
    }
};
REGISTER_WORKER("ElementMid", ElementMid)

/**
@brief ElementSink类
@details
    继承于Work类，重写了各个函数
*/

class ElementSink: public Element{
    public:
        ElementSink(){}
        ErrorCode initInternal(const std::string& json) override{return sophon_stream::common::ErrorCode::SUCCESS;}
        void uninitInternal() override{}
        void onStart()override{
            IVS_INFO("ElementSink start...");
        }
        void onStop()override{
            IVS_INFO("ElementSink stop...");
        }

        ErrorCode doWork() override{
            if(getDataCount(0)==0) return ErrorCode::SUCCESS;
            auto data = getData(0);
            popData(0);
            sendData(0, data, std::chrono::seconds(4));
            return ErrorCode::SUCCESS;
        }
};
REGISTER_WORKER("ElementSink", ElementSink)

/**
@brief ElementNew单元测试函数入口

@param [in] TestElement    测试用例命名
@param [in] OnepPipeLine  测试命名
@return void 无返回值


@UnitCase_ID
Element_UT_0014

@UnitCase_Name
unitcaseElementNew

@UnitCase_Description
实例化sourceElement，实例化midElement，实例化sinkElement，按顺序连接这三个Element，设置sinkElement端口0的回调函数，启动sourceElement、midElement和sinkElement，
sourceElement发送数据，sinkElement接收处理数据，并确认接收数据是否正确，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestElement, OnepPipeLine

@UnitCase_ExpectedResult
sourceElement、midElement和sinkElement能正常实例化和启动，并按照预定顺序先后连接，sourceElement正常发送数据，sinkElement正常接收处理数据，输入输出数据保持一致

*/

TEST(TestElement, OnepPipeLine) {
    auto& ElementFactory = SingletonElementFactory::getInstance();
    auto elementSource = ElementFactory.make("ElementSource");
    auto ElementMid = ElementFactory.make("ElementMid");
    auto ElementSink = ElementFactory.make("ElementSink");

    std::string strResult;
    std::mutex mtx;
    std::condition_variable cv;

    Element::connect(*elementSource, 0, *ElementMid, 0);
    Element::connect(*ElementMid, 0, *ElementSink, 0);
    ElementSink->setDataHandler(0,[&](std::shared_ptr<void> p){
            int result = *std::static_pointer_cast<int>(p);
            strResult += std::to_string(result);
            if(result==9){
                cv.notify_one();
            }
            });
    elementSource->start();
    ElementMid->start();
    ElementSink->start();
    for(int i=0;i<10;i++){
        std::shared_ptr<int> data = std::make_shared<int>(i);
        std::static_pointer_cast<ElementSource>(elementSource)->pushData(0, data, std::chrono::seconds(4));
    }
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    ElementSink->stop();
    ElementMid->stop();
    elementSource->stop();
    EXPECT_EQ(strResult, "0123456789");
}
