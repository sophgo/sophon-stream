/**
@file usecaseFaceDetectFeatureAttribute-Picture-withGrpc.cpp
@brief 主程序
@details
    FaceDetectFeatureAttribute-Picture-withGrpc集成测试

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

#include <opencv2/opencv.hpp>

/**
@brief 设置算法配置

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
@param [in] labelNames    标签名字

@return nlohmann::json 算法配置json数据
*/

nlohmann::json makeAlgorithmConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   std::vector<std::string> modelPath={""}, int maxBatchSize=1,
                                   std::vector<std::string> inputNodeName={""},
                                   std::vector<int> numInputs={1},
                                   std::vector<std::vector<int>> inputShape={{1}},
                                   std::vector<std::string> outputNodeName={""},
                                   std::vector<int> numOutputs={1},
                                   std::vector<std::vector<int>> outputShape={{1}},
                                   std::vector<float> threthold={0.5f},
                                   int numClass=1, 
                                   std::vector<std::string> labelNames={}) {
    nlohmann::json algorithmConfigure;
    try{
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
        //algorithmConfigure["label_names"] = {"age", "gender"};//举例 年龄性别属性
    }catch(std::exception&e){
        IVS_ERROR("{0}", e.what());
    }catch(...){
        IVS_ERROR("make json failed!");
    }
    return algorithmConfigure;
}

/**
@brief 设置编码配置

@param [in] sharedObject 算法so文件路径
@param [in] name       名字
@param [in] algorithmName 算法名字
@param [in] maxBatchSize  最大batch大小

@return nlohmann::json 编码配置json数据
*/

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

/**
@brief 设置Worker配置

@param [in] workerId worker号
@param [in] workerName    Worker名字
@param [in] side 设备类型
@param [in] deviceId     设备号
@param [in] threadNumber  线程数目
@param [in] timeout      
@param [in] repeatTimeout    
@param [in] batch    batch大小
@param [in] algoConfig  算法配置

@return nlohmann::json worker配置json数据
*/

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
    moduleConfigure["name"]=workerName;
    for(auto& ac:algoConfig) {
        moduleConfigure["models"].push_back(ac);
    }
    workerConf["configure"] = moduleConfigure;
    return workerConf;
}

/**
@brief 设置解码配置

@param [in] workerId worker号
@param [in] workerName    Worker名字
@param [in] side 设备类型
@param [in] deviceId     设备号
@param [in] threadNumber  线程数目
@param [in] timeout      
@param [in] repeatTimeout    
@param [in] batch    batch大小
@param [in] algoConfig  算法配置

@return nlohmann::json 解码配置json数据
*/

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

/**
@brief 设置module配置

@param [in] id          序号
@param [in] preWorkerIds    preWorker号
@param [in] moduleIds    module号
@param [in] postWorkerIds     postWorker号

@return nlohmann::json module配置json数据
*/

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

/**
@brief 设置connection配置

@param [in] srcId     输入worker号
@param [in] srcPort   输入端口号
@param [in] dstId     输出worker号
@param [in] dstPort   输出端口号

@return nlohmann::json connection配置json数据
*/

nlohmann::json makeConnectConfig(int srcId,int srcPort,int dstId,int dstPort) {
    nlohmann::json connectConf;
    connectConf["src_id"] = srcId;
    connectConf["src_port"] = srcPort;
    connectConf["dst_id"] = dstId;
    connectConf["dst_port"] = dstPort;
    return connectConf;
}

#define SOURCE_PICTURE_ID 5000
#define RETINA_FACE_ID 5005
#define FEATURE_FACE_ID 5006
#define ATTRIBUTE_FACE_ID 5007
#define ENCODE_ID 5008
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
            //filename.erase(filename.begin());
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
@brief FaceDetectFeatureAttribute-Picture-withGrpc集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
FaceDetectFeatureAttribute_IT_0003

@UnitCase_Name
usecaseFaceDetectFeatureAttribute-Picture-withGrpc

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
依次显示图片，在图片上检测出人脸并将对应的box绘制在相应位置，直到所有图片显示完成，程序可以正常退出

*/
#define TIMEOUT_TIME 0
TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    std::vector<std::string> paths;
    int ret = readFileList("../test/lynxi_face", paths);
    //int ret = readFileList("./123", paths);
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

//    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

    nlohmann::json graphConfigure;
    graphConfigure["graph_id"] = 1;
    nlohmann::json workersConfigure;


    nlohmann::json decodeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","picture_decoder","decode_picture", 
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
    { 0.3,0.4 },1);
    workersConfigure.push_back(makeWorkerConfig(SOURCE_PICTURE_ID, "action_worker", "nvidia", 0, 1, TIMEOUT_TIME, false, 1, {decodeJson}));

   // workersConfigure.push_back(makeWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1, {}));
    workersConfigure.push_back(makeWorkerConfig(REPORT_ID,"report_worker","host",0,1,TIMEOUT_TIME,false,1, {}));
    nlohmann::json retinafaceJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","retinaraceDetect","face_detect_retina",
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
    workersConfigure.push_back(makeWorkerConfig(RETINA_FACE_ID, "action_worker", "nvidia", 0, 1, TIMEOUT_TIME, true, 12, {retinafaceJson}));


    nlohmann::json alignJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","alignFace","face_align",
    {"../../share/models/"},1, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{3, 112, 112}}, {0.8},2,  {"faceAlign"});
    nlohmann::json featureJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","featureFace","face_feature",
    {"../../share/models/trtFaceFeature.txt"},35, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{512, 1, 1}}, {0.8},512,  {"faceFeature"});
    workersConfigure.push_back(makeWorkerConfig(FEATURE_FACE_ID,"action_worker","nvidia",0,1,TIMEOUT_TIME,true,35, {alignJson, featureJson}));


    nlohmann::json attributeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","attributeFace","face_attributes",
    {"../../share/models/trtAgeGender.txt"},35, {"data"}, {1}, {{3, 112, 112}},
    {"fc1"}, {1}, {{202, 1, 1}}, {0.8},2, {"gender", "age"});
    workersConfigure.push_back(makeWorkerConfig(ATTRIBUTE_FACE_ID,"action_worker","nvidia",0,1,TIMEOUT_TIME,true,35, {attributeJson}));
    
    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,TIMEOUT_TIME,true,1, {encodeJson}));

    graphConfigure["workers"] = workersConfigure;

    graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_ID}, {1}, {}));
    graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_FACE_ID, ATTRIBUTE_FACE_ID}, {}, {}));

    graphConfigure["connections"].push_back(makeConnectConfig(SOURCE_PICTURE_ID,0,0,0));
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
    auto reportOutput = graphOperateRequest.mutable_graph()->add_output_worker_ports();
    reportOutput->set_worker_id(REPORT_ID);
    reportOutput->set_worker_port(0);
    lynxi::ivs::grpc::GraphOperateResponse graphOperateResponse;
    ::grpc::Status operateGraphStatus = stub->OperateGraph(&operateGraphContext,
                                        graphOperateRequest,
                                        &graphOperateResponse);
#define THREAD_NUM 100
    std::vector<std::shared_ptr<std::thread>> vecThreads;
    for(int i=0;i<THREAD_NUM;i++){
        std::shared_ptr<std::thread> thread = std::make_shared<std::thread>([&paths, &stub](){
                   
                    while(true){
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
                            auto objectMetadata = objectProcessResponse.mutable_object_metadata();
                            if(objectMetadata->error_code()!=(int)lynxi::ivs::common::ErrorCode::SUCCESS){
                                IVS_ERROR("gtest decode picture failed!  {0}",objectMetadata->error_code() );
                            }
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
                                            IVS_DEBUG("callback feature:{0}", feature);

                                        }
                                }


                            }
                            
                    //        cv::imshow("123", cpuMat);
                    //        cv::waitKey(1);
                        }
                    }
                });
        vecThreads.push_back(thread);
    }

    for(int i=0;i<THREAD_NUM; i++){
        vecThreads[i]->join();
    }
    engineServer.stop();
    serverThread->join();
    IVS_DEBUG("engine server thread quit!");

}
