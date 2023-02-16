
#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "worker/MandatoryLink.h"
#include <opencv2/opencv.hpp>

nlohmann::json makeAlgorithmConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   std::vector<std::string> modelPath, int maxBatchSize,
                                   std::vector<std::string> inputNodeName,
                                   std::vector<int> numInputs,
                                   std::vector<std::vector<int>> inputShape,
                                   std::vector<std::string> outputNodeName,
                                   std::vector<int> numOutputs,
                                   std::vector<std::vector<int>> outputShape,
                                   std::vector<float> threthold,
                                   int numClass) {
    nlohmann::json algorithmConfigure;
    algorithmConfigure["shared_object"] = sharedObject;
    algorithmConfigure["name"] = name;
    algorithmConfigure["algorithm_name"] = algorithmName;
    algorithmConfigure["model_path"] = modelPath;
    algorithmConfigure["max_batchsize"] = maxBatchSize;
    algorithmConfigure["input_node_name"] = inputNodeName;
    algorithmConfigure["num_inputs"] = numInputs;
    algorithmConfigure["input_shape"] = inputShape;
    algorithmConfigure["output_node_name"] = outputNodeName;
    algorithmConfigure["num_outputs"] = numOutputs;
    algorithmConfigure["output_shape"] = outputShape;
    algorithmConfigure["threthold"] = threthold;
    algorithmConfigure["num_class"] = numClass;
    return algorithmConfigure;
}
nlohmann::json makeEncodeConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   int maxBatchSize
                                   ) {
    nlohmann::json algorithmConfigure;
    algorithmConfigure["shared_object"] = sharedObject;
    algorithmConfigure["name"] = name;
    algorithmConfigure["algorithm_name"] = algorithmName;
    algorithmConfigure["max_batchsize"] = maxBatchSize;
        return algorithmConfigure;
}

nlohmann::json makeWorkerConfig(int workerId,std::string workerName,
                                std::string side,int deviceId,
                                int threadNumber,int timeout,bool repeatTimeout,
                                int batch,
                                std::vector<nlohmann::json> algoConfig) {
    nlohmann::json workerConf;
    workerConf["id"] = workerId;
    workerConf["name"] = workerName;
    workerConf["side"] = side;
    workerConf["device_id"] = deviceId;
    workerConf["thread_number"] = threadNumber;
    workerConf["milliseconds_timeout"] = timeout;
    workerConf["repeated_timeout"] = repeatTimeout;

    nlohmann::json moduleConfigure;
    moduleConfigure["batch"] = batch;
    for(auto& ac:algoConfig) {
        moduleConfigure["models"].push_back(ac);
    }
    workerConf["configure"] = moduleConfigure;
    return workerConf;
}

nlohmann::json makeDecoderWorkerConfig(int workerId,std::string workerName,
                                std::string side,int deviceId,
                                int threadNumber,int timeout,bool repeatTimeout,
                                int batch,
                                 const std::string& soPath) {
    nlohmann::json workerConf;
    workerConf["id"] = workerId;
    workerConf["name"] = workerName;
    workerConf["side"] = side;
    workerConf["device_id"] = deviceId;
    workerConf["thread_number"] = threadNumber;
    workerConf["milliseconds_timeout"] = timeout;
    workerConf["repeated_timeout"] = repeatTimeout;

    nlohmann::json moduleConfigure;
    moduleConfigure["shared_object"] = soPath;
    workerConf["configure"] = moduleConfigure;
    return workerConf;
}
nlohmann::json makeModuleConfig(int id,std::vector<int> preWorkerIds,
                                std::vector<int> moduleIds,std::vector<int> postWorkerIds) {
    nlohmann::json config;
    config["id"] = id;
    for(auto& preWorkerId:preWorkerIds) {
        config["pre_worker_ids"].push_back(preWorkerId);
    }
    for(auto& moduleId:moduleIds) {
        config["module_ids"].push_back(moduleId);
    }
    for(auto& postWorker:postWorkerIds) {
        config["post_worker_ids"].push_back(postWorker);
    }
    return config;
}

nlohmann::json makeConnectConfig(int srcId,int srcPort,int dstId,int dstPort) {
    nlohmann::json connectConf;
    connectConf["src_id"] = srcId;
    connectConf["src_port"] = srcPort;
    connectConf["dst_id"] = dstId;
    connectConf["dst_port"] = dstPort;
    return connectConf;
}

#define DECODE_ID 5000
#define YOLO_ID 5001
#define REPORT_ID 5555

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");
    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;

    workersConfigure.push_back(makeWorkerConfig(DECODE_ID,"decoder_worker","host",0,1,200,true,1, {}));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,0,true,1, {}));

    nlohmann::json yoloJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","objectDetect","LynYoloV3",
    { "/home/xzl/lyngor_mm/tutorials/model_config/Yolov3" },
    1, { "data" }, { 1 }, {{3, 416, 416}}, { "output" }, { 1 }, { { 0, 0, 0 } },
    { 0.4,0.4 },80);
    workersConfigure.push_back(makeWorkerConfig(YOLO_ID,"action_worker","lyn",0,1,200,true,1, {yoloJson}));

    graphConfigure["workers"] = workersConfigure;

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,YOLO_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(YOLO_ID,0,REPORT_ID,0));

    std::mutex mtx;
    std::condition_variable cv;
    IVS_INFO("~~~~{0}",graphConfigure.dump());
    engine.addGraph(graphConfigure.dump());
    engine.setDataHandler(1,REPORT_ID,0,[&](std::shared_ptr<void> data) {

        auto objectMetadata = std::static_pointer_cast<lynxi::ivs::common::ObjectMetadata>(data);
        if(objectMetadata==nullptr) return;
        cv::Mat cpumat(objectMetadata->mFrame->mHeight,
                       objectMetadata->mFrame->mWidth, CV_8UC3,
                       objectMetadata->mFrame->mData.get());

        for(auto& submetadata:objectMetadata->mSubObjectMetadatas) {

            cv::rectangle(cpumat, cv::Rect(submetadata->mSpDataInformation->mBox.mX,
                                           submetadata->mSpDataInformation->mBox.mY,
                                           submetadata->mSpDataInformation->mBox.mWidth,
                                           submetadata->mSpDataInformation->mBox.mHeight),
                          cv::Scalar(0, 255, 0), 2);

        }
        cv::imshow(std::to_string(objectMetadata->getChannelId()),cpumat);
        cv::waitKey(1);
//        IVS_INFO("");

        if(objectMetadata->mFrame->mEndOfStream)
            cv.notify_one();
    });

    std::shared_ptr<lynxi::ivs::worker::ChannelTask> channelTask = std::make_shared<lynxi::ivs::worker::ChannelTask>();
    channelTask->request.channelId = 0;
    channelTask->request.operation = lynxi::ivs::worker::ChannelOperateRequest::ChannelOperate::START;
    nlohmann::json decodeConfigure;
    decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    decodeConfigure["resize_rate"] = 3.0f;
    decodeConfigure["timeout"] = 0;
    channelTask->request.json = decodeConfigure.dump();
    std::shared_ptr<void> data = std::static_pointer_cast<void>(channelTask);
    engine.sendData(1,DECODE_ID,0,data,std::chrono::seconds(4));
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
}
