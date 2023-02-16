
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

#define DECODE_ID 6000
#define RETINA_FACE_ID 6005
#define TRACK_WORK_ID 6006
#define QUALITY_WORK_ID 6007
#define FEATURE_FACE_ID 6008
#define ATTRIBUTE_FACE_ID 6009
#define ENCODE_ID 6010
#define REPORT_ID 6555

nlohmann::json makeTrackerConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   int maxBatchSize,float iou,int maxAge,int minHins, int updateTimes){
    nlohmann::json trackJson;
    trackJson["shared_object"] = sharedObject;
    trackJson["name"] = name;
    trackJson["algorithm_name"] = algorithmName;
    trackJson["max_batchsize"] = maxBatchSize;

    trackJson["track_Iou"] = iou;
    trackJson["track_MaxAge"] = maxAge;
    trackJson["track_MinHins"] = minHins;
    trackJson["track_UpdateTimes"] = updateTimes;
    return trackJson;

}

nlohmann::json makeQualityConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   int maxBatchSize,int topN, int baseTimes, float areaThresholds, 
      float lowBound, float upBound,float qualityWidth, float qualityHeight, float margin, float maxScore, float latertalSide){
    nlohmann::json qualityJson;
    qualityJson["shared_object"] = sharedObject;
    qualityJson["name"] = name;
    qualityJson["algorithm_name"] = algorithmName;
    qualityJson["max_batchsize"] = maxBatchSize;

    qualityJson["track_TopN"] = topN;
    qualityJson["track_BaseTimes"] = baseTimes;
    qualityJson["quality_ta"] = areaThresholds;
    qualityJson["quality_trl"] = lowBound;
    qualityJson["quality_tru"] = upBound;
    qualityJson["quality_w"] = qualityWidth;
    qualityJson["quality_h"] = qualityHeight;
    qualityJson["quality_mg"] = margin;
    qualityJson["quality_maxSc"] = maxScore;
    qualityJson["quality_ls"] = latertalSide;
    return qualityJson;

}

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;

    workersConfigure.push_back(makeDecoderWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1,"../lib/libmultiMediaApi.so" ));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,0,false,1, {}));
    nlohmann::json retinafaceJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","retinafaceDetect","face_detect_retina",
    { "../../share/models/retinaFaceVideo.txt" },
    12, { "data" }, { 1 }, {{3, 360, 640}},  {"face_rpn_cls_prob_reshape_stride32",
                                "face_rpn_bbox_pred_stride32",
                                "face_rpn_landmark_pred_stride32",
                                "face_rpn_cls_prob_reshape_stride16",
                                "face_rpn_bbox_pred_stride16",
                                "face_rpn_landmark_pred_stride16",
                                "face_rpn_cls_prob_reshape_stride8",
                                "face_rpn_bbox_pred_stride8",
                                "face_rpn_landmark_pred_stride8"
                               }, { 9}, {{4, 12, 20}, {8, 12, 20}, {20, 12, 20},
        {4, 23, 40}, {8, 23, 40}, {20, 23, 40},
        {4, 45, 80}, {8, 45, 80}, {20, 45, 80}
    },
    { 0.3,0.4 },1, {"faceDetect"});
    workersConfigure.push_back(makeWorkerConfig(RETINA_FACE_ID, "action_worker", "nvidia", 0, 1, 200, false, 8, {retinafaceJson}));

    nlohmann::json trackJson = makeTrackerConfig("../lib/libalgorithmApi.so","trackerSort","tracker_sort",
            1, 0.25f, 20, 3, 4);

    workersConfigure.push_back(makeWorkerConfig(TRACK_WORK_ID, "action_worker", "host", 0, 1, 200, false, 1, {trackJson}));

    nlohmann::json qualityJson = makeQualityConfig("../lib/libalgorithmApi.so","qualityFilter","quality_filter",
            1, 1, 3000000,1600.0f, 0.7f, 1.3f, 1920.0f,1080.0f, 30.0f, 19.0f, 112.0f );
    workersConfigure.push_back(makeWorkerConfig(QUALITY_WORK_ID, "action_worker", "host", 0, 1, 200, false, 1, {qualityJson}));

    nlohmann::json alignJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","alignFace","face_align",
    {"../../share/models/"},1, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{3, 112, 112}}, {0.8},2,  {"faceAlign"});
    nlohmann::json featureJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","featureFace","face_feature",
    {"../../share/models/trtFaceFeature.txt"},2, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{512, 1, 1}}, {0.8},512,  {"faceFeature"});
    workersConfigure.push_back(makeWorkerConfig(FEATURE_FACE_ID,"action_worker","nvidia",0,1,200,true,1, {alignJson, featureJson}));


    nlohmann::json attributeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","attributeFace","face_attributes",
    {"../../share/models/trtAgeGender.txt"},35, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{202, 1, 1}}, {0.8},2, {"gender", "age"});
    workersConfigure.push_back(makeWorkerConfig(ATTRIBUTE_FACE_ID,"action_worker","nvidia",0,1,200,true,1, {attributeJson}));

    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));
    
    graphConfigure["workers"] = workersConfigure;

    graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_ID, TRACK_WORK_ID, QUALITY_WORK_ID}, {1}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_FACE_ID, ATTRIBUTE_FACE_ID}, {}, {}));

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,0,0));
    graphConfigure["connections"].push_back(makeConnectConfig(0,0,ENCODE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID,0,REPORT_ID,0));

//    graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_ID}, {1}, {}));
//    graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_FACE_ID, ATTRIBUTE_FACE_ID}, {}, {}));
//
//    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,0,0));
//    graphConfigure["connections"].push_back(makeConnectConfig(0,0,REPORT_ID,0));

    std::mutex mtx;
    std::condition_variable cv;
    IVS_INFO("~~~~{0}",graphConfigure.dump());

    std::unique_ptr<lynxi::ivs::grpc::Engine::Stub> stub
        = lynxi::ivs::grpc::Engine::NewStub(::grpc::CreateChannel("localhost:50051",
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
        IVS_INFO("~~~~~~~~~~~~~~~~~~~~~recieved result!");
            if (report.object_metadata().packet().end_of_stream()) {
                cv.notify_one();
                clientReader->Finish();
            }
            if(report.object_metadata().packet().data().size()!=0){
                IVS_INFO("~~~~~~~report packet channel id:{0}", report.object_metadata().packet().channel_id());
                std::vector<uchar> inputarray;
                const std::string& strPacket = report.object_metadata().packet().data();
                for(int i=0;i<strPacket.size();i++){
                    inputarray.push_back(strPacket[i]);
                }
                IVS_INFO("usecaseFaceDetectFeatureAttribute-withGrpc  packet timestamp:{0}",report.object_metadata().packet().timestamp());
                cv::Mat cpuMat = cv::imdecode(inputarray,CV_LOAD_IMAGE_COLOR);
                int subObjLen = report.object_metadata().sub_object_metadatas_size();
                for(int i=0;i<subObjLen;i++){
                    auto subData = report.object_metadata().sub_object_metadatas(i);
                    IVS_INFO("x:{0}, y:{1},  w:{2}, h:{3}", subData.data_information().box().x(), 
                            subData.data_information().box().y(),subData.data_information().box().width(),
                            subData.data_information().box().height());
                    cv::rectangle(cpuMat, cv::Rect(subData.data_information().box().x(),
                                                   subData.data_information().box().y(),
                                                   subData.data_information().box().width(),
                                                   subData.data_information().box().height()),
                                  cv::Scalar(0, 255, 0), 2);
                    for(auto pair:subData.data_information().key_points()){
                        
                        cv::circle(cpuMat, cv::Point(pair.second.point().x(), pair.second.point().y()), 1, cv::Scalar(255, 255, 0), 1, 8, 0);
                    }


                    int subSubDataLen = subData.sub_object_metadatas_size();
                    for(int j=0;j<subSubDataLen;j++){
                        auto subSubData = subData.sub_object_metadatas(j);
                        int recognizeDataLen = subSubData.recognized_object_metadatas_size();
                            std::string strLabelName = subSubData.data_information().label_name();
                            if(strLabelName=="gender"){
                                std::string gender = subSubData.data_information().label();
                                int* iGender = (int*) gender.c_str();
                                std::string result = "gender: "+ std::to_string(*iGender);
                                cv::putText(cpuMat, result, cv::Point(subData.data_information().box().x(),
                                                   subData.data_information().box().y()), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                        1.5, cv::Scalar(255, 255, 0), 2);
                            }
                            else if(strLabelName=="age"){
                                std::string age = subSubData.data_information().label();
                                int* iAge = (int*) age.c_str();
                                std::string result = "age: "+ std::to_string(*iAge);
                                cv::putText(cpuMat, result, cv::Point(subData.data_information().box().x(),
                                                   subData.data_information().box().y()+subData.data_information().box().height()),
                                        cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                        1.5, cv::Scalar(255, 255, 0), 2);
                            }
                            else if(strLabelName=="faceFeature"){
                                std::string feature = subSubData.data_information().label();
                                //IVS_DEBUG("callback feature:{0}", feature);

                            }
//                        }
                    }


                }
                cv::imshow("123", cpuMat);
                cv::waitKey(1);
            }

        }
    });

    nlohmann::json decodeConfigure;
    //decodeConfigure["url"] = "../test/13.mp4";
    //decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    
    decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.24:554/Streaming/Channels/101?transportmode=unicast";
    //decodeConfigure["url"] = "../test/3.avi";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 2;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;
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
    channelOperateRequest.mutable_channel()->set_channel_id(1);
    IVS_INFO("decode config {0}",decodeConfigure.dump());
    channelOperateRequest.mutable_channel()->set_json_configure(decodeConfigure.dump());
    lynxi::ivs::grpc::ChannelOperateResponse channelOperateResponse;
    ::grpc::Status operateChannelStatus = stub->OperateChannel(&operateChannelContext,
                                          channelOperateRequest,
                                          &channelOperateResponse);
    IVS_DEBUG("after OperateChannel");

    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
    receiveThread->join();
}
