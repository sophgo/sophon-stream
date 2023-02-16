/**
@file unitcasePostModuleWorker.cpp
@brief 主程序
@details
    PostModuleWorker单元测试

@author lzw
@date 2020-06-17
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "framework/PostModuleWorker.cpp"
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
@brief PostModuleWorker单元测试函数入口

@param [in] TestPostModuleWorker  测试用例命名
@param [in] PostModuleWorker      测试命名
@return void 无返回值


@UnitCase_ID
PostModuleWorker_UT_0016

@UnitCase_Name
unitcasePostModuleWorker

@UnitCase_Description
实例化sourceWorker，实例化actionWorker，实例化PostModuleWorker，按顺序连接这三个Worker，初始化PostModuleWorker，启动actionWorker和PostModuleWorker，
sourceWorker发送数据，postModuleWorker接收处理数据，程序运行结束。

@UnitCase_Version
V0.1

@UnitCase_Precondition

@UnitCase_Input
TestPostModuleWorker, PostModuleWorker

@UnitCase_ExpectedResult
PostModuleWorker、actionWorker和sourceWorker能正常实例化和启动，sourceWorker正常发送数据，postModuleWorker正常接收处理数据，程序能正常退出

*/

TEST(TestPostModuleWorker, PostModuleWorker) {
    auto& workerFactory = SingletonWorkerFactory::getInstance();
    auto workerSource = workerFactory.make("WorkerSource");
    auto workerAction = workerFactory.make("WorkerAction");
    auto postModuleWorker = workerFactory.make("post_process_worker");
    
    std::mutex mtx;
    std::condition_variable cv;
   
    Worker::connect(*workerSource, 0, *workerAction, 0);
    Worker::connect(*workerAction, 0, *postModuleWorker, 0);
    postModuleWorker->setDataHandler(0,[&](std::shared_ptr<void> p){
        cv.notify_one();
    });

    std::string json = "{\"id\":1000, \"side\":\"cpu\", \"device_id\":0, \"thread_number\":1, \"milliseconds_timeout\":100,\"repeated_timeout\":true, \"configure\":{\"module_count\":2} }";
    ErrorCode ret = postModuleWorker->init(json);
    ASSERT_EQ(ret = ErrorCode::SUCCESS, ret);
    workerAction->start();
    postModuleWorker->start();
    for(int i=0;i<10;i++){
        std::shared_ptr<ObjectMetadata> data = std::make_shared<ObjectMetadata>();
        std::static_pointer_cast<WorkerSource>(workerSource)->pushData(0, data, std::chrono::seconds(4));
    }
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    workerAction->stop();
    postModuleWorker->stop();
}
