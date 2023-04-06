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
    objectMetadata->mFrame->mHandle = m_handle;
    objectMetadata->mFrame->mData.reset(new bm_device_mem_t, [&](bm_device_mem_t* p){
            bm_free_device(objectMetadata->mFrame->mHandle, *p);
            delete p;
        });
    
    if(img==nullptr){
        std::cout<<" img is null! "<<std::endl;
        objectMetadata->mFrame->mEndOfStream = true;
    }
    else{
        bm_image2Frame(objectMetadata->mFrame,*img);
        bm_malloc_device_byte(m_handle, objectMetadata->mFrame->mData.get(), objectMetadata->mFrame->mHeight
            * objectMetadata->mFrame->mWidth * objectMetadata->mFrame->mChannel * sizeof(float));
        bm_device_mem_t srcbm[3];
        bm_image_get_device_mem(*img,srcbm);
        bm_memcpy_d2d_byte(m_handle, *(objectMetadata->mFrame->mData), 0,srcbm[0],0,objectMetadata->mFrame->mHeight
           * objectMetadata->mFrame->mWidth * objectMetadata->mFrame->mChannel * sizeof(float));
        
        bm_image_destroy(*img);
            delete img;
        img = nullptr;
    }
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
