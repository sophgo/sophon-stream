#include "UnetPost.h"
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
namespace algorithm {
namespace post_process {

void UnetPost::init(algorithm::Context& context)
{

}

float UnetPost::sigmoid(float x)
{
  return 1.0 / (1 + expf(-x));
}


std::shared_ptr<common::Frame> UnetPost::bm_image2Frame(bm_handle_t&& handle,bm_image & img)
{
    std::shared_ptr<common::Frame> f = std::make_shared<common::Frame>();

    f->mData.reset(new bm_device_mem_t, [&](bm_device_mem_t* p){
        bm_free_device(handle, *p);
        delete p;
    });

    f->mWidth = img.width;
    f->mHeight = img.height;
    f->mDataType = common::data_bmcv2stream(img.data_type);
    f->mFormatType = common::format_bmcv2stream(img.image_format);  // GRAY 在stream里没有
    f->mChannel = 1;
    f->mDataSize = img.width * img.height * sizeof(uchar);

    bm_malloc_device_byte(handle, f->mData.get(), f->mHeight
     * f->mWidth * f->mChannel * sizeof(uchar));
    bm_device_mem_t srcbm[3];
    bm_image_get_device_mem(img,srcbm);
    bm_memcpy_d2d_byte(handle, *(f->mData), 0,srcbm[0],0,f->mHeight
     * f->mWidth * f->mChannel * sizeof(float));
    f->mHandle = handle;

    return f;
}


void UnetPost::postProcess(algorithm::Context& context,
    common::ObjectMetadatas& objectMetadatas)
    {
        context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
        std::shared_ptr<BMNNTensor> outputTensor = pSophgoContext->m_bmNetwork->outputTensor(0);
        int image_n = objectMetadatas.size();
        float outThresh = pSophgoContext->m_thresh[0];

        bmcv_resize_image bmcv_resize_attr;
        bmcv_resize_attr.roi_num = 1;
        bmcv_resize_attr.stretch_fit = 0;
        bmcv_resize_attr.padding_b = 0;
        bmcv_resize_attr.padding_g = 0;
        bmcv_resize_attr.padding_r = 0;

        bmcv_resize_t resize_img_attr;
        resize_img_attr.start_x = 0;
        resize_img_attr.start_y = 0;
        
        bmcv_resize_attr.interpolation = BMCV_INTER_NEAREST;
        for(int batch_idx = 0; batch_idx < image_n; ++batch_idx)
        {
            std::shared_ptr<common::Frame> image_ptr = objectMetadatas[batch_idx]->mFrame;
            int width = image_ptr->mWidth;
            int height = image_ptr->mHeight;
            std::shared_ptr<void> data = image_ptr->mData;

            bm_image result;
            // 取出一个输出tensor
            auto output_shape = outputTensor->get_shape();
            int feat_c = outputTensor->get_shape()->dims[1];
            int feat_h = outputTensor->get_shape()->dims[2];
            int feat_w = outputTensor->get_shape()->dims[3];
            int feature_size = feat_w * feat_h;
            uchar* decoded_data = new uchar[feature_size];
            float* tensor_data = outputTensor->get_cpu_data() + batch_idx * feat_c * feature_size;
            // tensor --> bm_img
            if (feat_c == 1)
            {
                for(int i = 0;i<feature_size;++i)
                    decoded_data[i] = sigmoid(tensor_data[i]) > outThresh ? 255 : 0;
            }
            else
            {
                for(int i = 0;i<feature_size;++i)
                {
                    // for num_classes > 2, argmax should be utilized
                    decoded_data[i] = (tensor_data[i] > tensor_data[i+feature_size]) ? 0 : 255;
                }
            }
            bm_image_create(pSophgoContext->m_bmContext->handle(), feat_h, feat_w, FORMAT_GRAY, DATA_TYPE_EXT_1N_BYTE, &result);
            bm_image_copy_host_to_device(result, (void**)(&decoded_data));
            
            bm_image result_resized;
            auto ret = bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, FORMAT_GRAY, DATA_TYPE_EXT_1N_BYTE, &result_resized);
            assert(BM_SUCCESS == ret);
            bm_image_alloc_dev_mem(result_resized);

            resize_img_attr.in_width = feat_w;
            resize_img_attr.in_height = feat_h;
            resize_img_attr.out_height = height;
            resize_img_attr.out_width = width;

            bmcv_resize_attr.resize_img_attr = & resize_img_attr;

            ret = bmcv_image_resize(pSophgoContext->m_bmContext->handle(), 1, &bmcv_resize_attr, &result, &result_resized);
            assert(BM_SUCCESS == ret);

            // save result to Metadata
            std::shared_ptr<common::SegmentedObjectMetadata> Segdata = std::make_shared<common::SegmentedObjectMetadata>();
            Segdata->mFrame = bm_image2Frame(pSophgoContext->m_bmContext->handle(), result_resized);
            objectMetadatas[batch_idx]->mSegmentedObjectMetadatas.emplace_back(Segdata);
            bm_image_destroy(result);
            delete [] decoded_data;
        }
        
        return;
    }
}
}
}