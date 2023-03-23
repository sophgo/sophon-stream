#include "SophgoDecode.h"
#include <opencv2/opencv.hpp>
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"

#include "bmruntime_interface.h"
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include "libyuv.h"
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace sophon_stream {
namespace multimedia {
namespace process {

//std::map<int, std::shared_ptr<decode::gpu::FFmpegHwDevice>> SophgoDecode::mSMapHwDevice;

SophgoDecode::SophgoDecode(){}

/**
 * 执行初始化
 * @param[in/out] context: 传输给推理模型的数据
 * @return 错误码
 */
common::ErrorCode SophgoDecode::init(multimedia::Context& context){
    mUrl = context.mUrl;
    mSourceType = context.mSourceType;
    mResizeRate = context.mResizeRate;
    int ret = bm_dev_request(&m_handle, context.deviceId);
    assert(BM_SUCCESS == ret);
    decoder.openDec(&m_handle,mUrl.c_str());

    // std::string strError;
    // common::ErrorCode errorCode = mSpFormatInput->init(mUrl, 10000, strError);
    // if(errorCode!=common::ErrorCode::SUCCESS) {
    //     IVS_ERROR("FormatInput init failed, errorcode: {0} info:{1}",
    //               (int)errorCode,strError);
    //     return errorCode;
    // }
    // mSpDecoder->setStream(mSpFormatInput->getStream());
    // decode::gpu::DecoderParam dp;
    // dp.resizeW = mSpFormatInput->getWidth()/mResizeRate;
    // dp.resizeH = mSpFormatInput->getHeight()/mResizeRate;

    // std::shared_ptr<decode::gpu::FFmpegHwDevice> spHwDevice = nullptr;
    // auto hwDeviceIter = mSMapHwDevice.find(context.deviceId); 
    // if(hwDeviceIter==mSMapHwDevice.end()){
    //     spHwDevice = std::make_shared<decode::gpu::FFmpegHwDevice>();
    //     errorCode = spHwDevice->init(context.deviceId);
    //     if(errorCode!=common::ErrorCode::SUCCESS) {
    //         IVS_ERROR("FFmpegHwDevice init failed, errorcode: {0}", (int)errorCode);
    //         return errorCode;
    //     }
    //     mSMapHwDevice.insert(std::make_pair(context.deviceId,spHwDevice));
    // }
    // else{
    //     spHwDevice = mSMapHwDevice[context.deviceId];
    // }
    
    // dp.pHwDev = spHwDevice.get();
    // errorCode = mSpDecoder->init(dp, mSpFormatInput->getWidth(), mSpFormatInput->getHeight(), "nvidia", context.deviceId,mSpFormatInput->getCodecId(), strError);

    // if(errorCode!=common::ErrorCode::SUCCESS) {
    //     IVS_ERROR("Decoder init failed, errorcode: {0} info:{1}",
    //               (int)errorCode,strError);
    //     return errorCode; 
    // }
    return common::ErrorCode::SUCCESS;
}

void bm_image2Frame(std::shared_ptr<common::Frame> & f,bm_image & img)
{

    f->mWidth = img.width;
    f->mHeight = img.height;
    f->mDataType = sophon_stream::common::data_bmcv2stream(img.data_type);
    f->mFormatType = sophon_stream::common::format_bmcv2stream(img.image_format);
    f->mChannel = 3;
    f->mDataSize = img.width * img.height * f->mChannel* sizeof(uchar);

    //f->mData = std::make_shared<void>(buffers);

}
/**
 * preprocess
 * @param[in] context: input and output config
 * @param[in] objectMetadatas: inputData
 * @return preprocess error code or common::ErrorCode::SUCCESS
 */
common::ErrorCode SophgoDecode::process(multimedia::Context& context,
        std::shared_ptr<common::ObjectMetadata>& objectMetadata) {

    context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
    if(pSophgoContext==nullptr) return (common::ErrorCode)(-1);

    // int flags = 0;
    // AVPacket* pkt = nullptr;
    // double timeStamp = 0.0;

    objectMetadata = std::make_shared<common::ObjectMetadata>();
    bm_image *img = decoder.grab();
    // static int a = 0;
    // char szpath[256] = {0}; 
    // sprintf(szpath,"out%d.bmp",a);
    // std::string strPath(szpath);
    // bm_image_write_to_bmp(*img, strPath.c_str());
    // a++;
    objectMetadata->mFrame = std::make_shared<common::Frame>();

    objectMetadata->mFrame->mData.reset(new bm_device_mem_t, [&](bm_device_mem_t* p){
            bm_free_device(m_handle, *p);
            delete p;
        });
    objectMetadata->mFrame->mHandle = m_handle;
    bm_image2Frame(objectMetadata->mFrame,*img);
    bm_malloc_device_byte(m_handle, objectMetadata->mFrame->mData.get(), objectMetadata->mFrame->mHeight
     * objectMetadata->mFrame->mWidth * objectMetadata->mFrame->mChannel * sizeof(float));
    bm_device_mem_t srcbm[3];
    bm_image_get_device_mem(*img,srcbm);
    bm_memcpy_d2d_byte(m_handle, *(objectMetadata->mFrame->mData), 0,srcbm[0],0,objectMetadata->mFrame->mHeight
     * objectMetadata->mFrame->mWidth * objectMetadata->mFrame->mChannel * sizeof(float));
    


    // common::ErrorCode errorCode = mSpFormatInput->readFrame(&pkt, mStrError, timeStamp, &flags);
    // //if(errorCode==common::ErrorCode::STREAM_END) exit(0);
    // int gotPicture = 0;
    // errorCode = mSpDecoder->decodePacket(mSpFormatInput->getStreamIndex(), pkt, gotPicture, mStrError, &timeStamp);
    // if(errorCode!=common::ErrorCode::SUCCESS&&errorCode!=common::ErrorCode::STREAM_END){
    //     return errorCode;
    // }
    // objectMetadata->mFrame = mSpDecoder->getOriginImage();
    // if(objectMetadata->mFrame==nullptr) return errorCode;

    //     objectMetadata->mSpDataInformation = std::make_shared<common::DataInformation>();
    //     if(pSophgoContext->mRoi.mWidth==0){
    //         objectMetadata->mSpDataInformation->mBox.mX = 0;
    //         objectMetadata->mSpDataInformation->mBox.mY = 0;
    //         objectMetadata->mSpDataInformation->mBox.mWidth = objectMetadata->mFrame->mWidth;
    //         objectMetadata->mSpDataInformation->mBox.mHeight = objectMetadata->mFrame->mHeight;
    //     }
    //     else{
    //         objectMetadata->mSpDataInformation->mBox = pSophgoContext->mRoi;
    //     }
    //     objectMetadata->mModelConfigureMap = nullptr;

    // if(objectMetadata->mFrame!=nullptr){
    //     objectMetadata->mFrame->mTimestamp = timeStamp*1000000;
    // }
    // else {
    //     IVS_ERROR("errorCode:{0},info:{1}",(int)errorCode,mStrError);
    // }

    // if(mSourceType==2){
    //     struct timeval timeVal;
    //     gettimeofday(&timeVal, nullptr);
    //     objectMetadata->mFrame->mTimestamp = (std::int64_t)timeVal.tv_sec*1000000+timeVal.tv_usec;
    // }
        
    // if(objectMetadata->mFrame!=nullptr&&errorCode==common::ErrorCode::STREAM_END) {
    //     objectMetadata->mFrame->mEndOfStream = true;
    // }

    // if(errorCode==common::ErrorCode::NOT_VIDEO_CHANNEL){
    //     return common::ErrorCode::SUCCESS;
    // }
    // return errorCode;



//    context::GpuContext* pSophgoContext = dynamic_cast<context::GpuContext*>(&context);
//    if(pSophgoContext==nullptr) return (common::ErrorCode)(-1);
//    pSophgoContext->numBatch = objectMetadatas.size();
//    for (int i = 0; i < pSophgoContext->numBatch; i++) {
//        auto &packet = objectMetadatas[i]->mPacket;
//        if (!packet||!packet->mData) {
//            if(packet==nullptr) std::cout<<"packet is null"<<std::endl;
//            std::cout<<"packet endofstream:"<<packet->mEndOfStream<<"---packet channel:"<<packet->mChannelId;
//            if(packet->mData==nullptr) std::cout<<"packet data is null"<<std::endl;
//            continue;
//        }
//
//        cv::Mat cpuMat;
//        std::vector<uchar> inputarray;
//        uchar* p = static_cast<uchar*>(packet->mData.get());
//        for(int i=0;i<packet->mDataSize;i++){
//            inputarray.push_back(p[i]);
//        }
//        cpuMat = cv::imdecode(inputarray, CV_LOAD_IMAGE_COLOR);
//        void* gpuData = nullptr;
//        int ret = cudaMalloc((void **)&gpuData, cpuMat.rows*cpuMat.cols*cpuMat.channels()* sizeof(uchar));
//        objectMetadatas[i]->mFrame = std::make_shared<common::Frame>();
//        objectMetadatas[i]->mFrame->mData.reset(gpuData,[](void* p) {
//            cudaFree(p);
//        });
//        cv::cuda::GpuMat gpuMat(cpuMat.rows, cpuMat.cols, CV_8UC3, objectMetadatas[i]->mFrame->mData.get());
//        gpuMat.upload(cpuMat);
//
//        objectMetadatas[i]->mFrame->mChannel = cpuMat.channels();
//        objectMetadatas[i]->mFrame->mWidth = cpuMat.cols;
//        objectMetadatas[i]->mFrame->mHeight = cpuMat.rows;
//        objectMetadatas[i]->mFrame->mFormatType = common::FormatType::BGR_PACKET;
//        objectMetadatas[i]->mFrame->mDataType = common::DataType::INTEGER;
//        objectMetadatas[i]->mFrame->mDataSize = objectMetadatas[i]->mFrame->mHeight*objectMetadatas[i]->mFrame->mWidth*objectMetadatas[i]->mFrame->mChannel;
//
//        objectMetadatas[i]->mFrame->mSide = "nvidia";
//        objectMetadatas[i]->mFrame->mWidthStep = sizeof(char)*objectMetadatas[i]->mFrame->mChannel;
//        objectMetadatas[i]->mFrame->mHeightStep = objectMetadatas[i]->mFrame->mWidthStep*objectMetadatas[i]->mFrame->mWidth;
//        objectMetadatas[i]->mFrame->mChannelStep = objectMetadatas[i]->mFrame->mHeightStep*objectMetadatas[i]->mFrame->mHeight;
//        objectMetadatas[i]->mFrame->mEndOfStream = packet->mEndOfStream;
//
//        objectMetadatas[i]->mSpDataInformation = std::make_shared<common::DataInformation>();
//        objectMetadatas[i]->mSpDataInformation->mBox.mX = 0;
//        objectMetadatas[i]->mSpDataInformation->mBox.mY = 0;
//        objectMetadatas[i]->mSpDataInformation->mBox.mWidth = objectMetadatas[i]->mFrame->mWidth;
//        objectMetadatas[i]->mSpDataInformation->mBox.mHeight = objectMetadatas[i]->mFrame->mHeight;
//
//        std::cout<<"~~~~~~~~~~~~~~preProcess SophgoDecode~~~~~~~~~~~~~~~~~"<<std::endl;
//
//    }
    return common::ErrorCode::SUCCESS;
}

/**
 * 执行uninit释放资源
 */
void SophgoDecode::uninit(){
    // if(mSpDecoder!=nullptr){
    //     mSpDecoder->destroy();
    // }
    // if(mSpFormatInput!=nullptr){
    //     mSpFormatInput->destroy();
    // }
}

} // namespace process
} // namespace multimedia
} // namespace sophon_stream
