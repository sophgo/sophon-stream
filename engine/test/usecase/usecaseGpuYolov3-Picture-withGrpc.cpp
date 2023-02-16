/**
@file usecaseGpuYolov3-Picture-withGrpc.cpp
@brief 主程序
@details
    GpuYolov3-Picture-withGrpc单元测试

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

#define SOURCE_PICTURE_ID 5000
#define YOLO_ID 5001
#define ENCODE_ID 5006
#define REPORT_ID 5555

#include <dirent.h>

/**
@brief 读取文件列表

@param [in] basePath   输入文件路径
@param [in] paths      遍历所有文件路径

@return int 1
*/

int readFileList(const char *basePath, std::vector<std::string> &paths) {
    DIR *dir;
    struct dirent *ptr;
    char base[1000] = {0};

    if ((dir = opendir(basePath)) == NULL) {
        perror("Open dir error...");
        exit(1);
    }
    char fullPath[2048] = {0};

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) ///current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8) {
            memset(fullPath, '\0', sizeof(char) * 2048);
            sprintf(fullPath, "%s/%s", basePath, ptr->d_name);
            std::string filename = std::string(fullPath);
            paths.push_back(filename);
            printf("file-d_name:%s\n", fullPath);
        } else if (ptr->d_type == 10) ///link file
            printf("d_name:%s/%s\n", basePath, ptr->d_name);
        else if (ptr->d_type == 4) { ///dir
            memset(base, '\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            readFileList(base, paths);
        }
    }
    closedir(dir);
    return 1;
}

/**
@brief GpuYolov3-Picture-withGrpc集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
GpuYolov3_IT_0011

@UnitCase_Name
usecaseGpuYolov3-Picture-withGrpc

@UnitCase_Description
依次经过解码、yolov3人脸检测、编码、输出worker，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
实例化EngineServer，启动EngineServer，给各个worker赋值，定义pipeline中各个worker的先后连接顺序，实例化一个客户端存根stub，
调用stub的OperateGraph和ReceiveReport方法，调用OperateChannel方法，通过clientReader的read方法接收处理数据，并实时显示结果

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestMultiAlgorithmGraph, MultiAlgorithmGraph

@UnitCase_ExpectedResult
依次显示图片，在图片上检测出人脸并将对应的box绘制在相应位置，等待按键输入然后继续显示下一张图片，直到所有图片显示完成，程序可以正常退出

*/

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    std::vector<std::string> paths;
    int ret = readFileList("../test/yolov3/pic_face", paths);
    if(ret!=1){
        IVS_ERROR("read dir error occurred!"); 
        return;
    }

    lynxi::ivs::grpc::EngineServer engineServer;
    std::shared_ptr<std::thread> serverThread
    = std::make_shared<std::thread>([&engineServer]() {
        engineServer.run("0.0.0.0:50050",
                         1024 * 1024 * 100,
                         1024 * 1024 * 100);
    });
    sleep(2);

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;

    nlohmann::json decodeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","picture_decoder","decode_picture",
    { "../../share/models/headYolov3.txt" },
    1, { "000_net" }, { 1 }, {{3, 1080, 1920}},  {"082_convolutional",
                                "094_convolutional",
                                "106_convolutional"
                               }, { 3}, {{18, 15, 25},{18, 30, 50},{18,60,100}
    },
    { 0.3,0.4 },1, {"headDetect"});
    workersConfigure.push_back(makeWorkerConfig(SOURCE_PICTURE_ID, "action_worker", "nvidia", 0, 1, 0, true, 1, {decodeJson}));

    // workersConfigure.push_back(makeWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1, {}));
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

    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);

    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));

    graphConfigure["workers"] = workersConfigure;

    graphConfigure["connections"].push_back(makeConnectConfig(SOURCE_PICTURE_ID,0,YOLO_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(YOLO_ID,0,ENCODE_ID,0));
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
    operateChannelOutput->set_worker_id(SOURCE_PICTURE_ID);
    operateChannelOutput->set_worker_port(1);
    auto reportOutput = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    reportOutput->set_worker_id(REPORT_ID);
    reportOutput->set_worker_port(0);
    lynxi::ivs::grpc::GraphOperateResponse graphOperateResponse;
    ::grpc::Status operateGraphStatus = stub->OperateGraph(&operateGraphContext,
                                        graphOperateRequest,
                                        &graphOperateResponse);

    for(auto&filename:paths){
        std::ifstream filePath(filename);
        std::string data;

        filePath >> std::noskipws;
        std::copy(std::istream_iterator<char>(filePath), std::istream_iterator<char>(), std::back_inserter(data));

        ::grpc::ClientContext processObjectContext;
        lynxi::ivs::grpc::ObjectProcessRequest objectProcessRequest;
        objectProcessRequest.set_graph_id(1);
        objectProcessRequest.mutable_request_worker_port()->set_worker_id(SOURCE_PICTURE_ID);
        objectProcessRequest.mutable_request_worker_port()->set_worker_port(0);
        objectProcessRequest.mutable_response_worker_port()->set_worker_id(REPORT_ID);
        objectProcessRequest.mutable_response_worker_port()->set_worker_port(0);
        objectProcessRequest.mutable_object_metadata()->mutable_packet()->set_channel_id(0);
        objectProcessRequest.mutable_object_metadata()->mutable_packet()->set_codec_type(lynxi::ivs::grpc::Packet::JPEG);
        objectProcessRequest.mutable_object_metadata()->mutable_packet()->set_end_of_stream(0);
        objectProcessRequest.mutable_object_metadata()->mutable_packet()->set_data(data);
        lynxi::ivs::grpc::ObjectProcessResponse objectProcessResponse;
        ::grpc::Status processObjectStatus = stub->ProcessObject(&processObjectContext, objectProcessRequest, &objectProcessResponse);
        if(!processObjectStatus.ok()){
            IVS_ERROR("grpc process error: {0}", processObjectStatus.error_message());
            continue;
        }
        IVS_DEBUG("grpc once send!");
        auto report = objectProcessResponse.mutable_object_metadata()->mutable_packet();
        std::vector<uchar> inputarray;
        const std::string& strPacket = report->data();
        for(int i=0;i<strPacket.size();i++){
            inputarray.push_back(strPacket[i]);
        }

        IVS_INFO("inputarray length:{0}", inputarray.size());

        cv::Mat cpuMat = cv::imdecode(inputarray,CV_LOAD_IMAGE_COLOR);
        int subObjLen = objectProcessResponse.mutable_object_metadata()->sub_object_metadatas_size();

        for(int i=0;i<subObjLen;i++){
            auto subData = objectProcessResponse.mutable_object_metadata()->sub_object_metadatas(i);
            IVS_INFO("x:{0}, y:{1},  w:{2}, h:{3}", subData.data_information().box().x(), 
                    subData.data_information().box().y(),subData.data_information().box().width(),
                    subData.data_information().box().height());
            cv::rectangle(cpuMat, cv::Rect(subData.data_information().box().x(),
                                            subData.data_information().box().y(),
                                            subData.data_information().box().width(),
                                            subData.data_information().box().height()),
                            cv::Scalar(0, 255, 0), 2);

            if (subData.data_information().label_name() == "headDetect")
            {
                std::string result = "score: "+std::to_string(subData.data_information().score());              
                cv::putText(cpuMat, result, cv::Point(subData.data_information().box().x(), subData.data_information().box().y()), 
                            cv::HersheyFonts::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(255, 255, 0), 2);
            }            
                 
        }
            
        cv::imshow("123", cpuMat);
        cv::waitKey(0);
    }

    engineServer.stop();
    serverThread->join();
    IVS_DEBUG("engine server thread quit!");
}
