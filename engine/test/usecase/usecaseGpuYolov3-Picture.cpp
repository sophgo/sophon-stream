/**
@file usecaseGpuYolov3-Picture.cpp
@brief 主程序
@details
    GpuYolov3-Picture集成测试

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
@brief GpuYolov3-Picture集成测试函数入口

@param [in] TestMultiAlgorithmGraph 测试用例命名
@param [in] MultiAlgorithmGraph     测试命名
@return void 无返回值


@UnitCase_ID
GpuYolov3_IT_0012

@UnitCase_Name
usecaseGpuYolov3-Picture

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

    auto& engine = lynxi::ivs::framework::SingletonEngine::getInstance();

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
                cv::rectangle(cpuMat, cv::Rect(detectData->mBox.mX, detectData->mBox.mY, detectData->mBox.mWidth, detectData->mBox.mHeight), 
                        cv::Scalar(0, 255, 0), 2);

                std::string labelName = detectData->mLabelName;
                if(labelName=="headDetect"){
                    std::string result = "score: "+std::to_string(detectData->mScore);
                    cv::putText(cpuMat, result, cv::Point(detectData->mBox.mX, detectData->mBox.mY), cv::HersheyFonts::FONT_HERSHEY_PLAIN, 
                            1.5, cv::Scalar(255, 255, 0), 2);
                }        
            }

            cv::imshow("headDetect", cpuMat);
            cv::waitKey(0);

            if(objectMetadata->mPacket->mEndOfStream) cv.notify_one();
        }
    });

    int pathsLen = paths.size();
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
        engine.sendData(1,SOURCE_PICTURE_ID,0,spData,std::chrono::seconds(4));
    }

    {
        std::unique_lock<std::mutex> uq(mtx);
        cv.wait(uq);
    }
}
