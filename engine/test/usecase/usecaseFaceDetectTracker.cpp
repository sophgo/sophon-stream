/**
@file usecaseFaceDetectTracker.cpp
@brief 主程序
@details
    FaceDetectTracker集成测试

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

#define DECODE_ID 5000
#define RETINA_FACE_ID 5005
#define TRACK_WORK_ID 5006
#define ENCODE_ID 5007
#define REPORT_ID 5555

/**
@brief 设置跟踪配置

@param [in] sharedObject 算法so文件路径
@param [in] name       名字
@param [in] algorithmName 算法名字
@param [in] maxBatchSize  最大batch大小
@param [in] topN     
@param [in] iou     
@param [in] maxAge    
@param [in] minHins  
@param [in] updateTimes    
@param [in] baseTimes  
@param [in] areaThresholds     
@param [in] lowBound      
@param [in] upBound    
@param [in] qualityWidth    
@param [in] qualityHeight   
@param [in] margin     
@param [in] maxScore      
@param [in] latertalSide    

@return nlohmann::json 跟踪配置json数据
*/

nlohmann::json makeTrackerConfig(const std::string& sharedObject,
                                   const std::string& name,const std::string& algorithmName,
                                   int maxBatchSize,int topN,float iou,int maxAge,int minHins, int updateTimes, int baseTimes, float areaThresholds, 
      float lowBound, float upBound,float qualityWidth, float qualityHeight, float margin, float maxScore, float latertalSide){
    nlohmann::json trackJson;
    trackJson["shared_object"] = sharedObject;
    trackJson["name"] = name;
    trackJson["algorithm_name"] = algorithmName;
    trackJson["max_batchsize"] = maxBatchSize;

    trackJson["track_TopN"] = topN;
    trackJson["track_Iou"] = iou;
    trackJson["track_MaxAge"] = maxAge;
    trackJson["track_MinHins"] = minHins;
    trackJson["track_UpdateTimes"] = updateTimes;
    trackJson["track_BaseTimes"] = baseTimes;
    trackJson["quality_ta"] = areaThresholds;
    trackJson["quality_trl"] = lowBound;
    trackJson["quality_tru"] = upBound;
    trackJson["quality_w"] = qualityWidth;
    trackJson["quality_h"] = qualityHeight;
    trackJson["quality_mg"] = margin;
    trackJson["quality_maxSc"] = maxScore;
    trackJson["quality_ls"] = latertalSide;
    return trackJson;

}

/**
@brief FaceDetectTracker集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
FaceDetectTracker_IT_0006

@UnitCase_Name
usecaseFaceDetectTracker

@UnitCase_Description
依次经过解码、retinaFace人脸检测、跟踪、编码、输出worker，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
具体先给各个worker赋值，定义pipeline中各个worker的先后连接顺序，然后添加graph并发送数据，接受数据并实时显示结果

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestMultiAlgorithmGraph, MultiAlgorithmGraph

@UnitCase_ExpectedResult
播放视频，在每一帧都会检测出人脸并进行跟踪，将对应的box绘制在相应位置，播放结束程序可以正常退出

*/

TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

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
            1, 1, 0.25f, 20, 3, 4, 3000000,400.0f, 0.7f, 1.3f, 640.0f,360.0f, 10.0f, 19.0f, 112.0f );

    IVS_CRITICAL("track json:{0}", trackJson.dump());

    workersConfigure.push_back(makeWorkerConfig(TRACK_WORK_ID, "action_worker", "host", 0, 1, 200, false, 1, {trackJson}));
    nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
    workersConfigure.push_back(makeWorkerConfig(ENCODE_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));
//    nlohmann::json alignJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","alignFace","face_align",
//    {"../../share/models/"},1, {"data"}, {1}, {{3, 112, 112}},
//    {"fc1"}, {1}, {{3, 112, 112}}, {0.8},2);
//    nlohmann::json featureJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","featureFace","face_feature",
//    {"../../share/models/trtFaceFeature.txt"},2, {"data"}, {1}, {{3, 112, 112}},
//    {"fc1"}, {1}, {{512, 1, 1}}, {0.8},512);
    //workersConfigure.push_back(makeWorkerConfig(FEATURE_FACE_ID,"action_worker","nvidia",0,1,200,true,1, {alignJson, featureJson}));


//    nlohmann::json attributeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","attributeFace","face_attributes",
//    {"../../share/models/trtAgeGender.txt"},35, {"data"}, {1}, {{3, 112, 112}},
//    {"fc1"}, {1}, {{202, 1, 1}}, {0.8},2);
    //workersConfigure.push_back(makeWorkerConfig(ATTRIBUTE_FACE_ID,"action_worker","nvidia",0,1,200,true,1, {attributeJson}));
    
    graphConfigure["workers"] = workersConfigure;


    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,RETINA_FACE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(RETINA_FACE_ID,0,TRACK_WORK_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(TRACK_WORK_ID,0,ENCODE_ID,0));
    graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_ID,0,REPORT_ID,0));

    std::mutex mtx;
    std::condition_variable cv;
    IVS_INFO("~~~~{0}",graphConfigure.dump());

    engine.addGraph(graphConfigure.dump());

    engine.setDataHandler(1,REPORT_ID,0,[&](std::shared_ptr<void> data) {

        IVS_DEBUG("data output 111111111111111");
        auto objectMetadata = std::static_pointer_cast<lynxi::ivs::common::ObjectMetadata>(data);
        if(objectMetadata==nullptr) {
            IVS_ERROR("Data output is null!");
            return;
        }

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
                cv::rectangle(cpuMat, cv::Rect(detectData->mBox.mX, detectData->mBox.mY, detectData->mBox.mWidth, detectData->mBox.mHeight), 
                        cv::Scalar(0, 255, 0), 2);

                for(auto pair:detectData->mKeyPoints){
                    
                    cv::circle(cpuMat, cv::Point(pair.second.mPoint.mX, pair.second.mPoint.mY), 1, cv::Scalar(255, 255, 0), 1, 8, 0);
                }
                cv::putText(cpuMat, std::to_string(objectMetadata->mSubObjectMetadatas[i]->mTrackedObjectMetadata->mTrackId), cv::Point(detectData->mBox.mX, detectData->mBox.mY), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                        1.5, cv::Scalar(255, 255, 0), 2);
                
                cv::putText(cpuMat, std::to_string(objectMetadata->mSubObjectMetadatas[i]->getTimestamp()), cv::Point(detectData->mBox.mX+detectData->mBox.mWidth, detectData->mBox.mY+detectData->mBox.mHeight),
                        cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                        1.5, cv::Scalar(255, 255, 0), 2);

                int subSubDataLen = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas.size();
                for(int j=0;j<subSubDataLen;j++){
                    auto subSubData = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas[j];
      //              int recognizeDataLen = subSubData->mRecognizedObjectMetadatas.size();
      //              for(int k=0;k<recognizeDataLen;k++){
      //                  auto recognizeData = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas[j]->mRecognizedObjectMetadatas[k];
      //                  if(recognizeData->mLabelName=="gender"){
      //                      std::string result = "gender: "+std::to_string(recognizeData->mTopKLabels[0]);
      //                      cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX, detectData->mBox.mY), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
      //                              1.5, cv::Scalar(255, 255, 0), 2);
      //                  }
      //                  else if(recognizeData->mLabelName=="age"){
      //                      std::string result = "age: "+std::to_string(recognizeData->mTopKLabels[0]);
      //                      cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX, detectData->mBox.mY+detectData->mBox.mHeight),
      //                              cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
      //                              1.5, cv::Scalar(255, 255, 0), 2);
      //                  }
      //                  else if(recognizeData->mLabelName=="faceFeature"){
      //                      std::string feature = recognizeData->mTopKLabelMetadatas[0]->mFeature;
      //                      IVS_DEBUG("callback feature:{0}", feature);

      //                  }
      //                  
      //              }
                }
            }

            cv::imshow("123", cpuMat);
            cv::waitKey(1);

            if(objectMetadata->mPacket->mEndOfStream) cv.notify_one();
        }






//        cv::Mat cpumat(objectMetadata->mFrame->mHeight,
//                       objectMetadata->mFrame->mWidth, CV_8UC3,
//                       objectMetadata->mFrame->mData.get());
//
//        for(auto& submetadata:objectMetadata->mSubObjectMetadatas) {
//
//            cv::rectangle(cpumat, cv::Rect(submetadata->mSpDataInformation->mBox.mX,
//                                           submetadata->mSpDataInformation->mBox.mY,
//                                           submetadata->mSpDataInformation->mBox.mWidth,
//                                           submetadata->mSpDataInformation->mBox.mHeight),
//                          cv::Scalar(0, 255, 0), 2);
//
//        }
//        cv::imshow(std::to_string(objectMetadata->getChannelId()),cpumat);
//        cv::waitKey(1);
////        IVS_INFO("");
//
//        if(objectMetadata->mFrame->mEndOfStream)
//            cv.notify_one();
    });
    nlohmann::json decodeConfigure;
    decodeConfigure["channel_id"] = 1;
    //decodeConfigure["url"] = "rtsp://lynxi:Lx20190812@192.168.17.24:554/Streaming/Channels/101?transportmode=unicast";
    decodeConfigure["url"] = "../test/13.mp4";
    //decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["multimedia_name"] = "decode_picture";
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
