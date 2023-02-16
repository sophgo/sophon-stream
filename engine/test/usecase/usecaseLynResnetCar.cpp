
#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "worker/MandatoryLink.h"
#include "common/ErrorCode.h"

#include <opencv2/opencv.hpp>

const int numClasses = 2;
std::string classes[numClasses] = {"car","bus"};

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
                                   int numClass, 
                                   std::vector<std::string> labelNames) {
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
    algorithmConfigure["label_names"] = labelNames;
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
#define RESNETCAR_ID 5001
#define ENCODE_ID 5006
#define REPORT_ID 5555

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;

    workersConfigure.push_back(makeWorkerConfig(DECODE_ID,"decoder_worker","host",0,1,0,false,1, {}));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,0,false,1, {}));
    nlohmann::json resnetJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","busCar","Resnet",
    { "../../share/models/lyn/Resnet50Car" },
    1, { "data" }, { 1 }, {{3, 224, 224}},  {"output"}, {1}, 
    {{0, 0, 0}},
    { 0,0 }, 2, {"busCar"});
    workersConfigure.push_back(makeWorkerConfig(RESNETCAR_ID, "action_worker", "lyn", 0, 1, 200, false, 8, {resnetJson}));
    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));

    graphConfigure["workers"] = workersConfigure;

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,RESNETCAR_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(RESNETCAR_ID,0,ENCODE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID,0,REPORT_ID,0));

    std::mutex mtx;
    std::condition_variable cv;
    IVS_INFO("~~~~{0}",graphConfigure.dump());

    engine.addGraph(graphConfigure.dump());

    engine.setDataHandler(1,REPORT_ID,0,[&](std::shared_ptr<void> data) {

        IVS_DEBUG("data output 111111111111111");
        auto objectMetadata = std::static_pointer_cast<lynxi::ivs::common::ObjectMetadata>(data);
        if(objectMetadata==nullptr) return;

        cv::Mat cpuMat;
        if(objectMetadata->mPacket!=nullptr&&objectMetadata->mPacket->mData!=nullptr){
            std::vector<uchar> inputarray;
            uchar* p = static_cast<uchar*>(objectMetadata->mPacket->mData.get());
            for(int i=0;i<objectMetadata->mPacket->mDataSize;i++){
                inputarray.push_back(p[i]);
            }
            cpuMat = cv::imdecode(inputarray, CV_LOAD_IMAGE_COLOR);
            for(int i=0;i<objectMetadata->mSubObjectMetadatas.size();i++){
                auto detectData = objectMetadata->mSubObjectMetadatas[i]->mSpDataInformation;

                std::string labelName = detectData->mLabelName;
                if(labelName=="busCar"){
                    int* label = (int*)detectData->mLabel.c_str();
                    std::string name = classes[*label];
                    std::cout <<"box类名："<<name << "，得分：" << detectData->mScore << std::endl;
                }        
            }

            if(objectMetadata->mPacket->mEndOfStream) cv.notify_one();
        }
    });

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test/yolov3/video/car_person.mp4";
    // decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
        
    auto channelTask = std::make_shared<lynxi::ivs::worker::ChannelTask>();
    channelTask->request.operation = lynxi::ivs::worker::ChannelOperateRequest::ChannelOperate::START;
    channelTask->request.json = decodeConfigure.dump();
    lynxi::ivs::common::ErrorCode errorCode = engine.sendData(1,
                                DECODE_ID,
                                0,
                                std::static_pointer_cast<void>(channelTask),
                                std::chrono::milliseconds(200));
    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
}
