#include "HostEncodePre.h"
#include <opencv2/opencv.hpp>
#include "../context/HostContext.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

/**
 * preprocess
 * @param[in] context: input and output config
 * @param[in] objectMetadatas: inputData
 * @return preprocess error code or common::ErrorCode::SUCCESS
 */
common::ErrorCode HostEncodePre::preProcess(algorithm::Context& context,
        common::ObjectMetadatas& objectMetadatas) {

    context::HostContext* pHostContext = dynamic_cast<context::HostContext*>(&context);
    if(pHostContext==nullptr) return (common::ErrorCode)(-1);
    pHostContext->numBatch = objectMetadatas.size();
    for (int i = 0; i < pHostContext->numBatch; i++) {
        auto &spMetadata = objectMetadatas[i];

//         if(spMetadata== nullptr) IVS_ERROR("report worker spmetadata is nullptr");
//         else if(spMetadata->mFrame==nullptr) IVS_ERROR("report worker spmetadata frame  is nullptr!");
//         else if(spMetadata->mFrame->mData==nullptr) IVS_ERROR("report worker spmetadata frame data is nullptr!");
// //        if(spMetadata->mErrorCode!=common::ErrorCode::SUCCESS){
// //            IVS_ERROR("encode get error, {0}", (int)spMetadata->mErrorCode );
// //        }
//         if(spMetadata!=nullptr&&spMetadata->mFrame!=nullptr&&spMetadata->mFrame->mData!=nullptr&&
//                 (spMetadata->mErrorCode==common::ErrorCode::SUCCESS||spMetadata->mErrorCode==common::ErrorCode::STREAM_END)){

//             //IVS_INFO("frame side:{0}", spMetadata->mFrame->mSide);
            
//             spMetadata->mPacket = std::make_shared<common::Packet>();
//             spMetadata->mPacket->mChannelId = spMetadata->mFrame->mChannelId;
//             spMetadata->mPacket->mTimestamp = spMetadata->mFrame->mTimestamp;
//             spMetadata->mPacket->mCodecType = common::CodecType::JPEG;
//             spMetadata->mPacket->mEndOfStream = spMetadata->mFrame->mEndOfStream;
//             spMetadata->mPacket->mSide = "host";
//             IVS_DEBUG("report worker, packet channel id:{0}", spMetadata->mPacket->mChannelId);
//             std::vector<uchar> buff;
//             std::vector<int> params = std::vector<int>(2);
//             if(spMetadata->mFrame->mSide=="nvidia"){
// //    #ifdef GPU_VERSION
//                 try{
//                     cv::cuda::GpuMat gpuMat(spMetadata->mFrame->mHeight, spMetadata->mFrame->mWidth, CV_8UC3, spMetadata->mFrame->mData.get());
//                     cv::Mat cpuMat;
//                     gpuMat.download(cpuMat);
//                     cv::imencode(".jpg",  cpuMat,  buff,  params);
//                 }
//                 catch(cv::Exception& e){
//                     IVS_ERROR("gpu2cpu or encode2jpg failed!{0}", e.what());
//                 }
// //    #else
// //    #endif
//             }
//             else{
//                 cv::Mat cpuMat(spMetadata->mFrame->mHeight, spMetadata->mFrame->mWidth, CV_8UC3, spMetadata->mFrame->mData.get());
//                 cv::imencode(".jpg", cpuMat ,  buff,  params);

//             }
//             if(buff.size()!=0){
//                 int bufferlen = buff.size();
//                 std::shared_ptr<uchar> packetData(new uchar[bufferlen], [](uchar* p){delete [] p;p=nullptr;});
//                 memcpy(packetData.get(), &buff[0], bufferlen*sizeof(uchar));
//                 spMetadata->mPacket->mData = std::static_pointer_cast<void>(packetData);
//                 spMetadata->mPacket->mDataSize = bufferlen*sizeof(uchar);
//                 spMetadata->mPacket->mTimestamp = spMetadata->mFrame->mTimestamp;
//             }
//         }

    }
    return common::ErrorCode::SUCCESS;
}

} // namespace pre_process
} // namespace algorithm
} // namespace sophon_stream
