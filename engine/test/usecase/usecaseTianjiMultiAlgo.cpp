
#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "worker/MandatoryLink.h"
#include "grpc/EngineServer.h"
#include "grpc/Engine.grpc.pb.h"

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
#define RESNET_CAR_ID 5002
#define RESNET_ANIMAL_ID 5003
#define REPORT_ID 5555

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    lynxi::ivs::grpc::EngineServer engineServer;
    std::shared_ptr<std::thread> serverThread
    = std::make_shared<std::thread>([&engineServer]() {
        engineServer.run("0.0.0.0:50050",
                         1024 * 1024 * 100,
                         1024 * 1024 * 100);
    });
    sleep(2);
//    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;

    workersConfigure.push_back(makeWorkerConfig(DECODE_ID,"decoder_worker","host",0,1,0,false,1, {}));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,0,false,1, {}));

    nlohmann::json yoloJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","objectDetect","LynYoloV3",
    { "/home/xzl/lyngor_mm/tutorials/model_config/Yolov3" },
    1, { "data" }, { 1 }, {{3, 416, 416}}, { "output" }, { 1 }, { { 0, 0, 0 } },
    { 0.4,0.4 },80);
    workersConfigure.push_back(makeWorkerConfig(YOLO_ID,"action_worker","lyn",0,1,200,true,1, {yoloJson}));

    nlohmann::json resnetCarJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","resnetCar","Resnet",
    {"/home/xzl/lyngor_mm/tutorials/model_config/Resnet50Car"},1, {"data"}, {1}, {{3, 224, 224}},
    {"output"}, {1}, {{0, 0, 0}}, {0,0},2);
    workersConfigure.push_back(makeWorkerConfig(RESNET_CAR_ID,"action_worker","lyn",0,1,200,true,1, {resnetCarJson}));

    nlohmann::json resnetAnimalJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","resnetAnimal","Resnet",
    {"/home/xzl/lyngor_mm/tutorials/model_config/Resnet50Dog"},1, {"data"}, {1}, {{3, 224, 224}},
    {"output"}, {1}, {{0, 0, 0}}, {0,0},2);
    workersConfigure.push_back(makeWorkerConfig(RESNET_ANIMAL_ID,"action_worker","lyn",0,1,200,true,1, {resnetAnimalJson}));

    graphConfigure["workers"] = workersConfigure;

    graphConfigure["modules"].push_back(makeModuleConfig(0, {YOLO_ID}, {1,2}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(1, {RESNET_CAR_ID}, {}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(2, {RESNET_ANIMAL_ID}, {}, {}));

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,0,0));
    graphConfigure["connections"].push_back(makeConnectConfig(0,0,REPORT_ID,0));

    std::mutex mtx;
    std::condition_variable cv;
    IVS_INFO("~~~~{0}",graphConfigure.dump());

    std::unique_ptr<lynxi::ivs::grpc::Engine::Stub> stub
        = lynxi::ivs::grpc::Engine::NewStub(::grpc::CreateChannel("localhost:50050",
                                            ::grpc::InsecureChannelCredentials()));


    ::grpc::ClientContext operateGraphContext;
    lynxi::ivs::grpc::GraphOperateRequest graphOperateRequest;
    graphOperateRequest.mutable_graph()->set_json_configure(graphConfigure.dump());
    auto operateChannelOutput = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    operateChannelOutput->set_worker_id(DECODE_ID);
    operateChannelOutput->set_worker_port(1);
    auto reportOutput = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    reportOutput->set_worker_id(REPORT_ID);
    reportOutput->set_worker_port(0);
    lynxi::ivs::grpc::GraphOperateResponse graphOperateResponse;
    ::grpc::Status operateGraphStatus = stub->OperateGraph(&operateGraphContext,
                                        graphOperateRequest,
                                        &graphOperateResponse);
//    engine.addGraph(graphConfigure.dump());

    ::grpc::ClientContext receiveReportContext;
    lynxi::ivs::grpc::ReceiveRequest reportReceiveRequest;
    reportReceiveRequest.set_graph_id(1);
    reportReceiveRequest.mutable_report_worker_port()->set_worker_id(REPORT_ID);
    reportReceiveRequest.mutable_report_worker_port()->set_worker_port(0);
    auto clientReader = stub->ReceiveReport(&receiveReportContext,
                                            reportReceiveRequest);

    std::shared_ptr<std::thread> receiveThread
    = std::make_shared<std::thread>([&clientReader, &cv]() {
        lynxi::ivs::grpc::Report report;
        while (clientReader->Read(&report)) {
            if (report.object_metadata().frame().end_of_stream()) {
                cv.notify_one();
                clientReader->Finish();
            }
            cv::Mat cpumat(report.object_metadata().frame().height(),
                           report.object_metadata().frame().width(), CV_8UC3,
                           (void*)report.object_metadata().frame().data().c_str());

            for(int i=0; i<report.object_metadata().sub_object_metadatas_size(); i++) {
                auto subdata = report.object_metadata().sub_object_metadatas(i);
                subdata.detected_object_metadata().box().x();

                cv::rectangle(cpumat, cv::Rect(subdata.detected_object_metadata().box().x(),
                                               subdata.detected_object_metadata().box().y(),
                                               subdata.detected_object_metadata().box().width(),
                                               subdata.detected_object_metadata().box().height()),
                              cv::Scalar(0, 255, 0), 2);
            }

            cv::imshow("123",cpumat);
            cv::waitKey(1);


        }
    });
//    engine.setDataHandler(1,
//                          REPORT_ID,
//                          0,
//    [&](std::shared_ptr<void> data) {
//        auto objectMetadata = std::static_pointer_cast<lynxi::ivs::common::ObjectMetadata>(data);
//        if(objectMetadata==nullptr) return;
////        try {
////            cv::Mat cpumat(objectMetadata->mFrame->mHeight,
////                           objectMetadata->mFrame->mWidth, CV_8UC3,
////                           objectMetadata->mFrame->mData.get());
//
////            for(auto& submetadata:objectMetadata->mSubObjectMetadatas) {
//
////                cv::rectangle(cpumat, cv::Rect(submetadata->mDetectedObjectMetadata->mBox.mX,
////                                               submetadata->mDetectedObjectMetadata->mBox.mY,
////                                               submetadata->mDetectedObjectMetadata->mBox.mWidth,
////                                               submetadata->mDetectedObjectMetadata->mBox.mHeight),
////                              cv::Scalar(0, 255, 0), 2);
//
////                IVS_CRITICAL("subRecogniz->mItemName size:{0}",submetadata->mRecognizedObjectMetadatas.size());
//////                for(auto& subRecogniz:submetadata->mRecognizedObjectMetadatas) {
//////                    if(submetadata->mRecognizedObjectMetadatas.size()==1) {
//////                        int i = 0;
//////                        i++;
//////                    }
//////                    IVS_CRITICAL("subRecogniz->mItemName:{0}",subRecogniz->mItemName);
////////                    cv::putText(cpumat,subRecogniz->mItemName,
////////                                cv::Point(submetadata->mDetectedObjectMetadata->mBox.mX,submetadata->mDetectedObjectMetadata->mBox.mY),
////////                                cv::HersheyFonts::FONT_HERSHEY_PLAIN,1.5,
////////                                cv::Scalar(255,255,0),2);
//////                }
//
////            }
////            cv::imshow(std::to_string(objectMetadata->getChannelId()),cpumat);
////            cv::waitKey(1);
////        } catch (cv::Exception& e) {
////            IVS_ERROR("cv error info :{0}",e.what());
////        }
//
//
//
//
//        IVS_INFO("");
//
//        if(objectMetadata->mFrame->mEndOfStream)
//            cv.notify_one();
//    });

//    std::shared_ptr<lynxi::ivs::worker::ChannelTask> channelTask = std::make_shared<lynxi::ivs::worker::ChannelTask>();
//    channelTask->request.channelId = 0;
//    channelTask->request.operation = lynxi::ivs::worker::ChannelOperateRequest::ChannelOperate::START;
    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 3.0f;
    decodeConfigure["timeout"] = 0;
//    channelTask->request.json = decodeConfigure.dump();
//    std::shared_ptr<void> data = std::static_pointer_cast<void>(channelTask);
//    engine.sendData(1,DECODE_ID,0,data,std::chrono::seconds(4));

    ::grpc::ClientContext operateChannelContext;
    lynxi::ivs::grpc::ChannelOperateRequest channelOperateRequest;
    channelOperateRequest.set_operate(lynxi::ivs::grpc::ChannelOperateRequest::ADD);
    channelOperateRequest.set_graph_id(1);
    channelOperateRequest.mutable_request_worker_port()->set_worker_id(DECODE_ID);
    channelOperateRequest.mutable_request_worker_port()->set_worker_port(0);
    channelOperateRequest.mutable_response_worker_port()->set_worker_id(DECODE_ID);
    channelOperateRequest.mutable_response_worker_port()->set_worker_port(1);
    IVS_INFO("decode config {0}",decodeConfigure.dump());
    channelOperateRequest.mutable_channel()->set_json_configure(decodeConfigure.dump());
    lynxi::ivs::grpc::ChannelOperateResponse channelOperateResponse;
    ::grpc::Status operateChannelStatus = stub->OperateChannel(&operateChannelContext,
                                          channelOperateRequest,
                                          &channelOperateResponse);

    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    engineServer.stop();
    receiveThread->join();
    serverThread->join();
}
