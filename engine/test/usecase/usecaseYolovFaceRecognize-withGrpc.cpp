/**
@file usecaseYolovFaceRecognize-withGrpc.cpp
@brief 主程序
@details
    YolovFaceRecognize-withGrpc集成测试

@author lzw
@date 2020-07-21
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "worker/MandatoryLink.h"
#include "common/ErrorCode.h"
#include "config.h"
#include "grpc/EngineServer.h"
#include "grpc/Engine.grpc.pb.h"
#include <opencv2/opencv.hpp>

/**
@brief 设置reID算法配置

@param [in] sharedObject 算法so文件路径
@param [in] name       名字
@param [in] algorithmName 算法名字
@param [in] modelPath     模型路径
@param [in] maxBatchSize  最大batch大小
@param [in] inputNodeName 输入节点名字    
@param [in] numInputs     输入个数
@param [in] inputShape    输入尺寸
@param [in] outputNodeName  输出节点名字
@param [in] numOutputs    输出个数
@param [in] outputShape   输出尺寸
@param [in] threthold     阈值
@param [in] numClass      分类种类数
@param [in] expandRatio   比例
@param [in] labelNames    标签名字

@return nlohmann::json 算法配置json数据
*/
nlohmann::json makeReIDAlgorithmConfig(const std::string& sharedObject,
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
                                   std::vector<float> expandRatio,
                                   std::vector<std::string> labelNames={}
                                   ) {
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
    algorithmConfigure["expand_ratio"] = expandRatio;
    return algorithmConfigure;
}

#define DECODE_ID 7000
#define YOLO_ID 7001
#define ENCODE_ID 7002
#define RETINA_FACE_ID 7003
#define FEATURE_FACE_ID 7004
#define FEATURE_PERSON_ID 7005
#define REPORT_ID 7777

/**
@brief GpuYolov3集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
GpuYolov3_IT_0014

@UnitCase_Name
usecaseGpuYolov3

@UnitCase_Description
依次经过解码、yolov3人脸检测、编码、输出worker，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
具体先给各个worker赋值，定义pipeline中各个worker的先后连接顺序，然后添加graph并发送数据，接受数据并实时显示结果

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestMultiAlgorithmGraph, MultiAlgorithmGraph

@UnitCase_ExpectedResult
播放视频，在每一帧都会检测出人脸并将对应的box绘制在相应位置，播放结束程序可以正常退出

*/

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

    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;

    workersConfigure.push_back(makeDecoderWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1, "../lib/libmultiMediaApi.so"));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,0,false,1, {}));
    nlohmann::json yolov3HeadJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","headDetect","YoloV3",
    { "../../share/models/headYolov3.txt" },
    1, { "000_net" }, { 1 }, {{3, 480, 800}},  {"082_convolutional",
                                "094_convolutional",
                                "106_convolutional"
                               }, { 3}, {{18, 15, 25},{18, 30, 50},{18,60,100}
    },
    { 0.3,0.4 },1, {"headDetect"});
    workersConfigure.push_back(makeWorkerConfig(YOLO_ID, "action_worker", "nvidia", 0, 1, 200, false, 8, {yolov3HeadJson}));

//    nlohmann::json reIdJson = makeAlgorithmConfig("../lib/libalgorithmApi.so", "personFeature", "person_reid", 
//            {"../../share/models/trtPersonReid.txt"},3, {"data"}, {1}, {{3, 256, 128}}, {"resnet0_batchnorm0"}, {1}, {{2048, 1, 1}}, {0.8}, 
//            2048, {"personFeature"} );
//    workersConfigure.push_back(makeWorkerConfig(RE_ID, "action_worker", "nvidia", 0, 1, 200, false, 8, {reIdJson}));

    nlohmann::json retinafaceJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","retinaFaceDetect","face_detect_retina",
    { "../../share/models/retinaFaceImage.txt" },
    12, { "data" }, { 1 }, {{3, 180, 180}},  {"face_rpn_cls_prob_reshape_stride32",
                                "face_rpn_bbox_pred_stride32",
                                "face_rpn_landmark_pred_stride32",
                                "face_rpn_cls_prob_reshape_stride16",
                                "face_rpn_bbox_pred_stride16",
                                "face_rpn_landmark_pred_stride16",
                                "face_rpn_cls_prob_reshape_stride8",
                                "face_rpn_bbox_pred_stride8",
                                "face_rpn_landmark_pred_stride8"
                               }, { 9}, {{4, 6, 6}, {8, 6, 6}, {20, 6, 6},
        {4, 12, 12}, {8, 12, 12}, {20, 12, 12},
        {4, 23, 23}, {8, 23, 23}, {20, 23, 23}
    },
    { 0.2,0.4 },1, {"faceDetect"});
    workersConfigure.push_back(makeWorkerConfig(RETINA_FACE_ID, "action_worker", "nvidia", 0, 1, 200, true, 12, {retinafaceJson}));

    nlohmann::json alignJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","alignFace","face_align",
    {"../../share/models/"},1, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{3, 112, 112}}, {0.8},2,  {"faceAlign"});
    nlohmann::json featureJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","featureFace","face_feature",
    {"../../share/models/trtFaceFeature.txt"},35, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{512, 1, 1}}, {0.8},512,  {"faceFeature"});
    workersConfigure.push_back(makeWorkerConfig(FEATURE_FACE_ID,"action_worker","nvidia",0,1,200,true,35, {alignJson, featureJson}));

    nlohmann::json reIDfeatureJson = makeReIDAlgorithmConfig("../lib/libalgorithmApi.so","personFeature","person_reid",
    {"../../share/models/trtPersonReid.txt"},3, {"data"}, {1}, {{3, 256, 128}},
    {"resnet0_batchnorm0"}, {1}, {{2048, 1, 1}}, {0.8},2048, {0.5, 0.0, 0.5, 2.5}, {"personFeature"});
    workersConfigure.push_back(makeWorkerConfig(FEATURE_PERSON_ID,"action_worker","nvidia",0,1,200,true,3, {reIDfeatureJson}));

    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));

    graphConfigure["workers"] = workersConfigure;

    graphConfigure["modules"].push_back(makeModuleConfig(0, {YOLO_ID}, {1}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_PERSON_ID, RETINA_FACE_ID}, {2}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(2, {FEATURE_FACE_ID}, {}, {}));

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,0,0));
    graphConfigure["connections"].push_back(makeConnectConfig(0,0,ENCODE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID,0,REPORT_ID,0));


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
                std::vector<uchar> inputarray;
                const std::string& strPacket = report.object_metadata().packet().data();
                for(int i=0;i<strPacket.size();i++){
                    inputarray.push_back(strPacket[i]);
                }
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

                    std::string labelName = subData.data_information().label_name();
                    if(labelName=="headDetect"){
                        std::string result = "score: "+std::to_string(subData.data_information().score());
                        cv::putText(cpuMat, result, cv::Point(subData.data_information().box().x(), subData.data_information().box().y()), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                1.5, cv::Scalar(255, 255, 0), 2);
                    }

                    for (auto subSubData:subData.sub_object_metadatas())
                    {
                        if (subSubData.data_information().label_name() == "personFeature")
                        {
                            IVS_INFO("reid feature:{0}", subSubData.data_information().label());
                        }
                        else if (subSubData.data_information().label_name() == "faceDetect")
                        {
                            IVS_INFO("label name:{0}", subSubData.data_information().label_name());
                            cv::rectangle(cpuMat, cv::Rect(subSubData.data_information().box().x(), subSubData.data_information().box().y(), 
                                subSubData.data_information().box().width(), subSubData.data_information().box().height()), cv::Scalar(0, 255, 0), 2);
                            for(auto pair:subSubData.data_information().key_points()){
                                cv::circle(cpuMat, cv::Point(pair.second.point().x(), pair.second.point().y()), 1, cv::Scalar(255, 255, 0), 1, 8, 0);
                            }

                            for(auto& subSubSubData:subSubData.sub_object_metadatas()){
                                IVS_INFO("face feature:{0}", subSubSubData.data_information().label());
                            }                          
                        }
                    }
                }
                cv::imshow("123", cpuMat);
                cv::waitKey(1);
            }

        }
    });

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;

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
