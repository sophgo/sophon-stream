/**
@file usecaseFaceDetectTrackerQFFeatureAttributes.cpp
@brief 主程序
@details
    FaceDetectTrackerQFFeatureAttributes集成测试

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
#define RETINA_FACE_VIDEO_ID 5005
#define TRACK_WORK_VIDEO_ID 5006
#define QUALITY_WORK_VIDEO_ID 5007
#define FEATURE_FACE_VIDEO_ID 5008
#define ATTRIBUTE_FACE_VIDEO_ID 5009
#define ENCODE_VIDEO_ID 5010
#define REPORT_VIDEO_ID 5555




#define SOURCE_PICTURE_ID 5000
#define RETINA_FACE_PIC_ID 5005
#define FEATURE_FACE_PIC_ID 5006
#define ATTRIBUTE_FACE_PIC_ID 5007
#define ENCODE_PIC_ID 5008
#define REPORT_PIC_ID 5555



/**
@brief 设置跟踪配置

@param [in] sharedObject 算法so文件路径
@param [in] name       名字
@param [in] algorithmName 算法名字
@param [in] maxBatchSize  最大batch大小    
@param [in] iou     
@param [in] maxAge    
@param [in] minHins  
@param [in] updateTimes    

@return nlohmann::json 跟踪配置json数据
*/

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

/**
@brief 设置质量配置

@param [in] sharedObject 算法so文件路径
@param [in] name       名字
@param [in] algorithmName 算法名字
@param [in] maxBatchSize  最大batch大小
@param [in] topN         
@param [in] baseTimes  
@param [in] areaThresholds     
@param [in] lowBound      
@param [in] upBound    
@param [in] qualityWidth    
@param [in] qualityHeight   
@param [in] margin     
@param [in] maxScore      
@param [in] latertalSide    

@return nlohmann::json 质量配置json数据
*/

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
@brief FaceDetectTrackerQFFeatureAttributes集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
FaceDetectTrackerQFFA_IT_0009

@UnitCase_Name
usecaseFaceDetectTrackerQFFeatureAttributes

@UnitCase_Description
依次经过解码、retinaFace人脸检测、跟踪、质量过滤、人脸特征、人脸属性、编码、输出worker，检测结果存储在输入objectMetadata的mSubObjectMetadatas字段下的mSpDataInformation中。
具体先给各个worker赋值，定义pipeline中各个worker的先后连接顺序，然后添加graph并发送数据，接受数据并实时显示结果

@UnitCase_Version
V0.1

@UnitCase_Precondition
models文件为本地文件，没有随工程一起上传，需要在对应目录放置models文件夹，包括models文件夹中应该按照目录放置对应显卡的模型文件

@UnitCase_Input
TestMultiAlgorithmGraph, MultiAlgorithmGraph

@UnitCase_ExpectedResult
播放视频，在每一帧都会检测人脸进行跟踪并进行质量过滤，同时提取人脸特征和人脸属性，将对应的box和属性绘制在相应位置，播放结束程序可以正常退出

*/

#define GRAPH_ID_VIDEO 2
#define GRAPH_ID_PIC 1
TEST(TestMultiAlgorithmGraph, MultiAlgorithmGraph) {
    ivs::logInit("debug","","");

    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();
    std::string strGraphVideo;
    std::string strGraphPicture;

    {
        nlohmann::json graphConfigure;
        graphConfigure["graph_id"] = GRAPH_ID_VIDEO;
        nlohmann::json workersConfigure;

        workersConfigure.push_back(makeDecoderWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1, "../lib/libmultiMediaApi.so"));
        workersConfigure.push_back(makeWorkerConfig(REPORT_VIDEO_ID,"report_worker","host",0,1,0,false,1, {}));
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
        workersConfigure.push_back(makeWorkerConfig(RETINA_FACE_VIDEO_ID, "action_worker", "nvidia", 0, 1, 200, false, 8, {retinafaceJson}));

        nlohmann::json trackJson = makeTrackerConfig("../lib/libalgorithmApi.so","trackerSort","tracker_sort",
                1, 0.25f, 20, 3, 4);

        workersConfigure.push_back(makeWorkerConfig(TRACK_WORK_VIDEO_ID, "action_worker", "host", 0, 1, 200, false, 1, {trackJson}));

        nlohmann::json qualityJson = makeQualityConfig("../lib/libalgorithmApi.so","qualityFilter","quality_filter",
                1, 1, 3000000,3600.0f, 0.7f, 1.3f, 1920.0f,1080.0f, 30.0f, 19.0f, 112.0f );
        workersConfigure.push_back(makeWorkerConfig(QUALITY_WORK_VIDEO_ID, "action_worker", "host", 0, 1, 200, false, 1, {qualityJson}));
        
        nlohmann::json alignJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","alignFace","face_align",
        {"../../share/models/"},1, {"data"}, {1}, {{3, 112, 112}},
        {"fc1"}, {1}, {{3, 112, 112}}, {0.8},2,  {"faceAlign"});
        nlohmann::json featureJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","featureFace","face_feature",
        {"../../share/models/trtFaceFeature.txt"},2, {"data"}, {1}, {{3, 112, 112}},
        {"fc1"}, {1}, {{512, 1, 1}}, {0.8},512,  {"faceFeature"});
        workersConfigure.push_back(makeWorkerConfig(FEATURE_FACE_VIDEO_ID,"action_worker","nvidia",0,1,200,true,1, {alignJson, featureJson}));


        nlohmann::json attributeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","attributeFace","face_attributes",
        {"../../share/models/trtAgeGender.txt"},35, {"data"}, {1}, {{3, 112, 112}},
        {"fc1"}, {1}, {{202, 1, 1}}, {0.8},2, {"gender", "age"});
        workersConfigure.push_back(makeWorkerConfig(ATTRIBUTE_FACE_VIDEO_ID,"action_worker","nvidia",0,1,200,true,1, {attributeJson}));
        nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
        workersConfigure.push_back(makeWorkerConfig(ENCODE_VIDEO_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));
        
        graphConfigure["workers"] = workersConfigure;
        
        graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_VIDEO_ID, TRACK_WORK_VIDEO_ID, QUALITY_WORK_VIDEO_ID}, {1}, {}));
        graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_FACE_VIDEO_ID, ATTRIBUTE_FACE_VIDEO_ID}, {}, {}));

        graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,0,0));
        graphConfigure["connections"].push_back(makeConnectConfig(0,0, ENCODE_VIDEO_ID,0));
        graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_VIDEO_ID,0,REPORT_VIDEO_ID,0));
    //    graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_ID, TRACK_WORK_ID}, {1}, {}));
    //    graphConfigure["modules"].push_back(makeModuleConfig(1, {QUALITY_WORK_ID}, {}, {}));

    //    graphConfigure["connections"].push_back(makeConnectConfig(DECODE_ID,0,RETINA_FACE_ID,0));
    //    graphConfigure["connections"].push_back(makeConnectConfig(RETINA_FACE_ID,0,TRACK_WORK_ID,0));
    //    graphConfigure["connections"].push_back(makeConnectConfig(TRACK_WORK_ID,0,QUALITY_WORK_ID,0));
    //    graphConfigure["connections"].push_back(makeConnectConfig(QUALITY_WORK_ID,0,REPORT_ID,0));

        IVS_INFO("~~~~{0}",graphConfigure.dump());
        strGraphVideo = graphConfigure.dump();
    }

    {
        nlohmann::json graphConfigure;
        graphConfigure["graph_id"] = GRAPH_ID_PIC;
        nlohmann::json workersConfigure;


        nlohmann::json decodeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","picture_decoder","decode_picture", 
        { "" },
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
        workersConfigure.push_back(makeWorkerConfig(SOURCE_PICTURE_ID, "action_worker", "nvidia", 0, 1, 0, true, 1, {decodeJson}));

       // workersConfigure.push_back(makeWorkerConfig(DECODE_ID,"decoder_worker","nvidia",0,1,0,false,1, {}));
        workersConfigure.push_back(makeWorkerConfig(REPORT_PIC_ID,"report_worker","host",0,1,0,false,1, {}));
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
        workersConfigure.push_back(makeWorkerConfig(RETINA_FACE_PIC_ID, "action_worker", "nvidia", 0, 1, 200, false, 8, {retinafaceJson}));


        nlohmann::json alignJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","alignFace","face_align",
        {"../../share/models/"},1, {"data"}, {1}, {{3, 112, 112}},
        {"fc1"}, {1}, {{3, 112, 112}}, {0.8},2,  {"faceAlign"});
        nlohmann::json featureJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","featureFace","face_feature",
        {"../../share/models/trtFaceFeature.txt"},2, {"data"}, {1}, {{3, 112, 112}},
        {"fc1"}, {1}, {{512, 1, 1}}, {0.8},512,  {"faceFeature"});
        workersConfigure.push_back(makeWorkerConfig(FEATURE_FACE_PIC_ID,"action_worker","nvidia",0,1,200,true,1, {alignJson, featureJson}));


        nlohmann::json attributeJson = makeAlgorithmConfig("../lib/libalgorithmApi.so","attributeFace","face_attributes",
        {"../../share/models/trtAgeGender.txt"},35, {"data"}, {1}, {{3, 112, 112}},
        {"fc1"}, {1}, {{202, 1, 1}}, {0.8},2, {"gender", "age"});
        workersConfigure.push_back(makeWorkerConfig(ATTRIBUTE_FACE_PIC_ID,"action_worker","nvidia",0,1,200,true,1, {attributeJson}));
        
        nlohmann::json encodeJson = makeEncodeConfig("../lib/libalgorithmApi.so","","encode_picture",1);
        workersConfigure.push_back(makeWorkerConfig(ENCODE_PIC_ID,"action_worker","host",0,1,200,true,1, {encodeJson}));

        graphConfigure["workers"] = workersConfigure;

        graphConfigure["modules"].push_back(makeModuleConfig(0, {RETINA_FACE_PIC_ID}, {1}, {}));
        graphConfigure["modules"].push_back(makeModuleConfig(1, {FEATURE_FACE_PIC_ID, ATTRIBUTE_FACE_PIC_ID}, {}, {}));

        graphConfigure["connections"].push_back(makeConnectConfig(SOURCE_PICTURE_ID,0,0,0));
        graphConfigure["connections"].push_back(makeConnectConfig(0,0,ENCODE_PIC_ID,0));
        graphConfigure["connections"].push_back(makeConnectConfig(ENCODE_PIC_ID,0,REPORT_PIC_ID,0));

        strGraphPicture = graphConfigure.dump();

    }

    std::mutex mtx;
    std::condition_variable cv;

    engine.addGraph(strGraphVideo);
    engine.addGraph(strGraphPicture);

    engine.setDataHandler(GRAPH_ID_VIDEO,REPORT_VIDEO_ID,0,[&](std::shared_ptr<void> data) {

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

                cv::Mat crop = cpuMat(cv::Rect(detectData->mBox.mX, detectData->mBox.mY, detectData->mBox.mWidth, detectData->mBox.mHeight));

                static int var = 0;
                std::string cropPath = "/home/xzl/delete/111/";
                cropPath+=std::to_string(objectMetadata->mSubObjectMetadatas[i]->mTrackedObjectMetadata->mTrackId);
                cropPath+="_";
                cropPath+=std::to_string(var);
                cropPath+="_";
                cropPath+=std::to_string(objectMetadata->mSubObjectMetadatas[i]->getTimestamp());
                cropPath+=".jpg";
                IVS_DEBUG("crop path:{0}", cropPath);
                cv::imwrite(cropPath, crop);
                var++;


                int subSubDataLen = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas.size();
                for(int j=0;j<subSubDataLen;j++){
                    auto subSubData = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas[j];
                    std::string strLabelName = subSubData->mSpDataInformation->mLabelName;
                    if(strLabelName=="gender"){
                        std::string gender = subSubData->mSpDataInformation->mLabel;
                        int* iGender = (int*) gender.c_str();
                        std::string result = "gender: "+ std::to_string(*iGender);
                        cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX,
                                           detectData->mBox.mY), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                1.5, cv::Scalar(255, 255, 0), 2);
                    }
                    else if(strLabelName=="age"){
                        std::string age = subSubData->mSpDataInformation->mLabel;
                        int* iAge = (int*) age.c_str();
                        std::string result = "age: "+ std::to_string(*iAge);
                        cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX,
                                           detectData->mBox.mY+detectData->mBox.mHeight),
                                cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                1.5, cv::Scalar(255, 255, 0), 2);
                    }
                    else if(strLabelName=="faceFeature"){
                        std::string feature = subSubData->mSpDataInformation->mLabel;
                        IVS_DEBUG("callback feature:{0}", feature);

                    }
                }
            }

//            cv::imshow("123", cpuMat);
//            cv::waitKey(1);

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


    engine.setDataHandler(GRAPH_ID_PIC,REPORT_PIC_ID,0,[&](std::shared_ptr<void> data) {

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
                cv::rectangle(cpuMat, cv::Rect(detectData->mBox.mX, detectData->mBox.mY, detectData->mBox.mWidth, detectData->mBox.mHeight), 
                        cv::Scalar(0, 255, 0), 2);
                int subSubDataLen = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas.size();
                for(int j=0;j<subSubDataLen;j++){
                    auto subSubData = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas[j];
                    std::string labelName = subSubData->mSpDataInformation->mLabelName;
                    if(labelName=="gender"){
                        int* iGender = (int*)subSubData->mSpDataInformation->mLabel.c_str();
                        std::string result = "gender: "+std::to_string(*iGender);
                        cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX, detectData->mBox.mY), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                1.5, cv::Scalar(255, 255, 0), 2);
                    }
                    else if(labelName=="age"){
                        int* iAge = (int*)subSubData->mSpDataInformation->mLabel.c_str();
                        std::string result = "age: "+std::to_string(*iAge);
                        cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX, detectData->mBox.mY+detectData->mBox.mHeight),
                                cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                                1.5, cv::Scalar(255, 255, 0), 2);
                    }
                    else if(labelName=="faceFeature"){
                        std::string feature = subSubData->mSpDataInformation->mLabel;
                        IVS_DEBUG("callback feature:{0}", feature);

                    }
      //              int recognizeDataLen = subSubData->mRecognizedObjectMetadatas.size();
      //              for(int k=0;k<recognizeDataLen;k++){
      //                  auto recognizeData = objectMetadata->mSubObjectMetadatas[i]->mSubObjectMetadatas[j]->mRecognizedObjectMetadatas[k];
      //                  
      //              }
                }
            }

//            try{
//                cv::imshow("123", cpuMat);
//                cv::waitKey(1);
//            }
//            catch(cv::Exception& e){
//                IVS_ERROR("imshow exception :{0}", e.what());
//            }

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
    //decodeConfigure["url"] = "../test/13.mp4";
    //decodeConfigure["url"] = "../test/2.mp4";
    decodeConfigure["url"] =    "rtmp://192.168.3.20:1935/live/instance0";//"rtmp://192.168.3.209:1935/live/instance1";
    
    //decodeConfigure["url"] = "../test/3.avi";
    //decodeConfigure["url"] = "../test/out-720p-2s.mp4";
    //decodeConfigure["url"] = "../test/18.mp4";
    decodeConfigure["resize_rate"] = 2.0f;
    decodeConfigure["timeout"] = 0;
    decodeConfigure["source_type"] = 0;
    decodeConfigure["reopen_times"] = -1;
    decodeConfigure["multimedia_name"] = "decode_picture";
        
    auto channelTask = std::make_shared<lynxi::ivs::worker::ChannelTask>();
    channelTask->request.operation = lynxi::ivs::worker::ChannelOperateRequest::ChannelOperate::START;
    channelTask->request.json = decodeConfigure.dump();
    lynxi::ivs::common::ErrorCode errorCode = engine.sendData(GRAPH_ID_VIDEO,
                                DECODE_ID,
                                0,
                                std::static_pointer_cast<void>(channelTask),
                                std::chrono::milliseconds(200));


    //picture recognization

    std::vector<std::string> paths;
    int ret = readFileList("../test/lynxi_face", paths);
    if(ret!=1){
        IVS_ERROR("read dir error occurred!"); 
        return;
    }
    
    int pathsLen = paths.size();
    while(true){
        for(int i=0;i<paths.size();i++){
            std::ifstream filePath(paths[i]);
            std::string data;

            filePath >> std::noskipws;
            std::copy(std::istream_iterator<char>(filePath), std::istream_iterator<char>(), std::back_inserter(data));

            std::shared_ptr<lynxi::ivs::common::ObjectMetadata> spData = std::make_shared<lynxi::ivs::common::ObjectMetadata>();
            spData->mPacket = std::make_shared<lynxi::ivs::common::Packet>();
            spData->mPacket->mCodecType = lynxi::ivs::common::CodecType::JPEG;
            spData->mPacket->mChannelId = 0;
            if(i==pathsLen-1){
                spData->mPacket->mEndOfStream = true;
            }
            else{
                spData->mPacket->mEndOfStream = false;
            }
            spData->mPacket->mDataSize = data.length();
            std::shared_ptr<char> spJpgData(new char[spData->mPacket->mDataSize], [](char* p){delete [] p;});
            memcpy(spJpgData.get(),data.c_str() , spData->mPacket->mDataSize);
            spData->mPacket->mData = std::static_pointer_cast<void>(spJpgData);
            engine.sendData(GRAPH_ID_PIC,SOURCE_PICTURE_ID,0,spData,std::chrono::seconds(4));

        }
    }
    engine.stop(GRAPH_ID_VIDEO);
    engine.removeGraph(GRAPH_ID_VIDEO);
    engine.stop(GRAPH_ID_PIC);
    engine.removeGraph(GRAPH_ID_PIC);

}
