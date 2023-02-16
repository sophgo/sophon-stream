/**
@file usecaseFaceDetectFeatureAttribute-withGrpc.cpp
@brief 主程序
@details
    FaceDetectFeatureAttribute-withGrpc集成测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "worker/MandatoryLink.h"
#include "grpc/EngineServer.h"
#include "grpc/Engine.grpc.pb.h"
#include "config.h"

#include <opencv2/opencv.hpp>


#define DECODE_ID 6000
#define RETINA_FACE_ID 6005
#define FEATURE_FACE_ID 6006
#define ATTRIBUTE_FACE_ID 6007
#define ENCODE_ID 6008
#define REPORT_ID 6555

#define GRAPH_ID 9

/**
@brief FaceDetectFeatureAttribute-withGrpc集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
FaceDetectFeatureAttribute_IT_0005

@UnitCase_Name
usecaseFaceDetectFeatureAttribute-withGrpc

@UnitCase_Description
依次经过解码、retinaFace人脸检测、人脸对齐、人脸特征提取、人脸属性分析、编码、输出worker，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
实例化EngineServer，启动EngineServer，给各个worker赋值，定义pipeline中各个worker的先后连接顺序，实例化一个客户端存根stub，
调用stub的OperateGraph和ReceiveReport方法，调用OperateChannel方法，通过clientReader的read方法接收处理数据，并实时显示结果

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
//    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = GRAPH_ID;
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

    graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_ID}, {1}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_FACE_ID, ATTRIBUTE_FACE_ID}, {}, {}));

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
    graphOperateRequest.set_operate(lynxi::ivs::grpc::GraphOperateRequest::ADD);

    auto operateChannelOutput = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    operateChannelOutput->set_worker_id(DECODE_ID);
    operateChannelOutput->set_worker_port(1);

    auto reportOutput = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    reportOutput->set_worker_id(REPORT_ID);
    reportOutput->set_worker_port(0);


//    auto channelOutput0 = graphOperateRequest.mutable_graph()->add_output_worker_ports();
//    channelOutput0->set_worker_id(DECODE_ID);
//    channelOutput0->set_worker_port(0);


    auto channelOutput2 = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    channelOutput2->set_worker_id(DECODE_ID);
    channelOutput2->set_worker_port(2);

    lynxi::ivs::grpc::GraphOperateResponse graphOperateResponse;
    ::grpc::Status operateGraphStatus = stub->OperateGraph(&operateGraphContext,
                                        graphOperateRequest,
                                        &graphOperateResponse);
//    engine.addGraph(graphConfigure.dump());

    ::grpc::ClientContext receiveReportContext;
    lynxi::ivs::grpc::ReceiveRequest reportReceiveRequest;
    reportReceiveRequest.set_graph_id(GRAPH_ID);
    reportReceiveRequest.mutable_report_worker_port()->set_worker_id(REPORT_ID);
    reportReceiveRequest.mutable_report_worker_port()->set_worker_port(0);
    auto clientReader = stub->ReceiveReport(&receiveReportContext,
                                            reportReceiveRequest);

    std::shared_ptr<std::thread> receiveThread
    = std::make_shared<std::thread>([&clientReader, &cv]() {
        lynxi::ivs::grpc::Report report;
        while (clientReader->Read(&report)) {
        IVS_INFO("~~~~~~~~~~~~~~~~~~~~~recieved result!");
        static int count = 0;
        count++;
        if(count>700) {
            cv.notify_one();
            clientReader->Finish();
            return;
        }
            if (report.object_metadata().packet().end_of_stream()) {
                cv.notify_one();
                clientReader->Finish();
                return;
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
    decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.24:554/Streaming/Channels/101?transportmode=unicast";
    //decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    //decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.12:554/Streaming/Channels/101?transportmode=unicast";

    //decodeConfigure["url"] = "../test/3.avi";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
//    channelTask->request.json = decodeConfigure.dump();
//    std::shared_ptr<void> data = std::static_pointer_cast<void>(channelTask);
//    engine.sendData(1,DECODE_ID,0,data,std::chrono::seconds(4));

    ::grpc::ClientContext operateChannelContext;
    lynxi::ivs::grpc::ChannelOperateRequest channelOperateRequest;
    channelOperateRequest.set_operate(lynxi::ivs::grpc::ChannelOperateRequest::ADD);
    channelOperateRequest.set_graph_id(GRAPH_ID);
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


//    nlohmann::json removeGraphJson;
//    removeGraphJson["graph_id"] = GRAPH_ID;
//
//    ::grpc::ClientContext operateGraphContextRemove;
//    lynxi::ivs::grpc::GraphOperateRequest graphOperateRequestRemove;
//    graphOperateRequestRemove.mutable_graph()->set_json_configure(removeGraphJson.dump());
//    graphOperateRequestRemove.set_operate(lynxi::ivs::grpc::GraphOperateRequest::REMOVE);
//    lynxi::ivs::grpc::GraphOperateResponse graphOperateResponseRemove;
//    ::grpc::Status operateGraphStatusRemove = stub->OperateGraph(&operateGraphContextRemove,
//                                        graphOperateRequestRemove,
//                                        &graphOperateResponseRemove);

    engineServer.stop();
    receiveThread->join();
    serverThread->join();

}
