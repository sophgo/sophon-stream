/**
@file unitcaseWorkerManager.cpp
@brief 主程序
@details
    WorkerManager单元测试

@author lzw
@date 2020-06-18
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/WorkerFactory.h"
#include "framework/WorkerManager.h"
#include "worker/MandatoryLink.h"
#include "common/Logger.h"
#include "../usecase/config.h"
#include "common/ErrorCode.h"

using namespace lynxi::ivs::common;
using namespace lynxi::ivs::framework;

#define DECODE_ID 5000
#define YOLO_ID 5001
#define ENCODE_ID 5006
#define REPORT_ID 5555

/**
@brief WorkerManager单元测试函数入口

@param [in] TestWorkerManager 测试用例命名
@param [in] OnepPipeLine      测试命名
@return void 无返回值


@UnitCase_ID
Worker_UT_0017

@UnitCase_Name
unitcaseWorkerManager

@UnitCase_Description
实例化WorkerManager，分别测试init、getSideAndDeviceId、setDataHandler、start、sendData、pause、resume、stop和uninit。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestWorkerManager, OnepPipeLine

@UnitCase_ExpectedResult
能正常实例化WorkerManager，并调用init、getSideAndDeviceId等成员方法。

*/

TEST(TestWorkerManager, OnepPipeLine) {
    auto workerManager = std::make_shared<WorkerManager>();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;
    workersConfigure.push_back(makeDecoderWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1, "../lib/libmultiMediaApi.so"));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,0,false,1, {}));
    nlohmann::json yolov3HeadJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","headDetect","YoloV3",{"../../share/models/headYolov3.txt"},1,{"000_net"},
    {1},{{3,480,800}},{"082_convolutional","094_convolutional","106_convolutional"}, {3}, {{18,15,25},{18,30,50},{18,60,100}},{0.3,0.4},1,{"headDetect"});
    workersConfigure.push_back(makeWorkerConfig(YOLO_ID, "action_worker", "nvidia", 0, 1, 200, false, 8, {yolov3HeadJson}));
    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));
    graphConfigure["workers"] = workersConfigure;
    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,YOLO_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(YOLO_ID,0,ENCODE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID,0,REPORT_ID,0));

    ErrorCode ret = workerManager->init(graphConfigure.dump());
    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    std::pair<std::string, int> sideAndDeviceId = workerManager->getSideAndDeviceId(YOLO_ID);
    ASSERT_EQ(sideAndDeviceId.first, "nvidia");
    ASSERT_EQ(sideAndDeviceId.second, YOLO_ID);

    std::mutex mtx;
    std::condition_variable cv;
    workerManager->setDataHandler(REPORT_ID, 0, [&](std::shared_ptr<void> p) {
        IVS_DEBUG("data output");
    });

    ret = workerManager->start();
    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;
    auto channelTask = std::make_shared<lynxi::ivs::worker::ChannelTask>();
    channelTask->request.operation = lynxi::ivs::worker::ChannelOperateRequest::ChannelOperate::START;
    channelTask->request.json = decodeConfigure.dump();

    ret = workerManager->sendData(DECODE_ID, 0, std::static_pointer_cast<void>(channelTask), std::chrono::milliseconds(200));
    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    ret = workerManager->pause();
    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    ret = workerManager->resume();
    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    ret = workerManager->stop();
    ASSERT_EQ(ret, ErrorCode::SUCCESS);

    workerManager->uninit();
}