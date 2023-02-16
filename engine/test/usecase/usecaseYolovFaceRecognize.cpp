/**
@file usecaseGpuYolov3.cpp
@brief 主程序
@details
    GpuYolov3集成测试

@author lzw
@date 2020-06-12
@version A001
@copyright Lynxi Technologies Co., Ltd
*/

#include "gtest/gtest.h"
#include "common/Logger.h"
#include "framework/Engine.h"
#include "worker/MandatoryLink.h"
#include "common/ErrorCode.h"
#include "config.h"

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
    graphConfigure["modules"].push_back(makeModuleConfig(1, {RETINA_FACE_ID}, {2}, {FEATURE_PERSON_ID}));
    graphConfigure["modules"].push_back(makeModuleConfig(2, {FEATURE_FACE_ID}, {}, {}));

    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,0,0));
    graphConfigure["connections"].push_back(makeConnectConfig(0,0,ENCODE_ID,0));
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
                auto subMetadata = objectMetadata->mSubObjectMetadatas[i];
                auto detectData = subMetadata->mSpDataInformation;
                cv::rectangle(cpuMat, cv::Rect(detectData->mBox.mX, detectData->mBox.mY, detectData->mBox.mWidth, detectData->mBox.mHeight), 
                        cv::Scalar(0, 255, 0), 2);

                // cv::putText(cpuMat, std::to_string(objectMetadata->mSubObjectMetadatas[i]->getTimestamp()), cv::Point(detectData->mBox.mX+detectData->mBox.mWidth, detectData->mBox.mY+detectData->mBox.mHeight),
                //         cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                //         1.5, cv::Scalar(255, 255, 0), 2);

                std::string labelName = detectData->mLabelName;
                if(labelName=="headDetect"){
                    std::string result = "score: "+std::to_string(detectData->mScore);
                    cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX, detectData->mBox.mY), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                            1.5, cv::Scalar(255, 255, 0), 2);
                }
                for(auto& subSubdata:subMetadata->mSubObjectMetadatas){
                    if(subSubdata->mSpDataInformation->mLabelName=="personFeature"){
                        IVS_INFO("reid feature:{0}", subSubdata->mSpDataInformation->mLabel);
                    }
                    else if(subSubdata->mSpDataInformation->mLabelName=="faceDetect"){
                        //IVS_INFO("label name:{0}", subSubdata->mSpDataInformation->mLabelName);
                        cv::rectangle(cpuMat, cv::Rect(subSubdata->mSpDataInformation->mBox.mX, subSubdata->mSpDataInformation->mBox.mY, subSubdata->mSpDataInformation->mBox.mWidth, subSubdata->mSpDataInformation->mBox.mHeight), 
                            cv::Scalar(0, 255, 0), 2);
                        for(auto pair:subSubdata->mSpDataInformation->mKeyPoints){
                            
                            cv::circle(cpuMat, cv::Point(pair.second.mPoint.mX, pair.second.mPoint.mY), 1, cv::Scalar(255, 255, 0), 1, 8, 0);
                        }

                        for(auto& subSubSubdata:subSubdata->mSubObjectMetadatas){
                            IVS_INFO("face feature:{0}", subSubSubdata->mSpDataInformation->mLabel);
                        }
                    }

                    IVS_INFO("label name : {0}", subSubdata->mSpDataInformation->mLabelName);
                    
                }
            }

            cv::imshow("headDetect", cpuMat);
            cv::waitKey(1);

            if(objectMetadata->mPacket->mEndOfStream) cv.notify_one();
        }
    });

    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    //decodeConfigure["url"] = "../test/13.mp4";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
    decodeConfigure["reopen_times"] = -1;
        
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
