#include "UnetPre.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"


namespace sophon_stream {
namespace algorithm {
namespace pre_process {

#define DUMP_FILE 1
#define USE_ASPECT_RATIO 1

common::ErrorCode UnetPre::preProcess(algorithm::Context& context,
    common::ObjectMetadatas & objectMetadatas){
        context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
        int image_n = objectMetadatas.size();
        // std::vector<std::shared_ptr<common::Frame>> images(image_n);
        std::shared_ptr<BMNNTensor> input_tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
        bmcv_resize_image resize_attr;
        int ret = 0;
        for(int i = 0;i < image_n; ++i)
        {
            // 从objectMetadata读取frame信息
            pSophgoContext->m_frame_w = objectMetadatas[i]->mFrame->mWidth;
            pSophgoContext->m_frame_h = objectMetadatas[i]->mFrame->mHeight;
            int width = objectMetadatas[i]->mFrame->mWidth;
            int height = objectMetadatas[i]->mFrame->mHeight;
            // 转格式
            // sophon_stream::common::FormatType format_type_stream = objectMetadatas[i]->mFrame->mFormatType;
            // sophon_stream::common::DataType data_type_stream = objectMetadatas[i]->mFrame->mDataType;
            // bm_image_format_ext format_type_bmcv = common::format_stream2bmcv(format_type_stream);
            // bm_image_data_format_ext data_type_bmcv = common::data_stream2bmcv(data_type_stream);
            // 转成bm_image
            // bm_image image1 = *objectMetadatas[i]->mFrame->mSpData;
            // bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, format_type_bmcv, data_type_bmcv, &image1);
            // bm_image_attach(image1, objectMetadatas[i]->mFrame->mSpData.get());

            // BGR_PACKED转PLANAR
            bm_image image;
            bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, FORMAT_BGR_PLANAR, DATA_TYPE_EXT_1N_BYTE, &image);
            bmcv_image_storage_convert(pSophgoContext->m_bmContext->handle(), 1, objectMetadatas[i]->mFrame->mSpData.get(), &image);

            bm_image image_aligned;
            bool need_copy = image.width & (64-1);
            if (need_copy)
            {
                int stride1[3], stride2[3];
                bm_image_get_stride(image, stride1);
                stride2[0] = FFALIGN(stride1[0], 64);
                stride2[1] = FFALIGN(stride1[1], 64);
                stride2[2] = FFALIGN(stride1[2], 64);
                bm_image_create(pSophgoContext->m_bmContext->handle(), image.height, image.width, 
                                FORMAT_BGR_PLANAR, DATA_TYPE_EXT_1N_BYTE, &image_aligned, stride2);
                bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
                bmcv_copy_to_atrr_t copyToAttr;
                memset(&copyToAttr, 0, sizeof(copyToAttr));
                copyToAttr.start_x = 0;
                copyToAttr.start_y = 0;
                copyToAttr.if_padding = 1;
                bmcv_image_copy_to(pSophgoContext->m_bmContext->handle(), copyToAttr, image, image_aligned);
            }
            else
            {
                image_aligned = image;
            }
#if USE_ASPECT_RATIO
            bool isAlignWidth = false;
            float ratio = get_aspect_scaled_ratio(width, height, pSophgoContext->m_net_w, pSophgoContext->m_net_h, &isAlignWidth);
            bmcv_padding_atrr_t padding_attr;
            memset(&padding_attr, 0, sizeof(padding_attr));
            padding_attr.dst_crop_sty = 0;
            padding_attr.dst_crop_stx = 0;
            padding_attr.padding_b = 114;
            padding_attr.padding_g = 114;
            padding_attr.padding_r = 114;
            padding_attr.if_memset = 1;
            if (isAlignWidth) 
            {
                padding_attr.dst_crop_h = height*ratio;
                padding_attr.dst_crop_w = pSophgoContext->m_net_w;
                int ty1 = (int)((pSophgoContext->m_net_h - padding_attr.dst_crop_h) / 2);
                padding_attr.dst_crop_sty = ty1;
                padding_attr.dst_crop_stx = 0;
            }
            else
            {
                padding_attr.dst_crop_h = pSophgoContext->m_net_h;
                padding_attr.dst_crop_w = width*ratio;
                int tx1 = (int)((pSophgoContext->m_net_w - padding_attr.dst_crop_w) / 2); // 0
                padding_attr.dst_crop_sty = 0;
                padding_attr.dst_crop_stx = tx1;
            }
            bmcv_rect_t crop_rect {0, 0, width, height};
            // pSophgoContext->m_resized_imgs需要在Inference::init里初始化
            auto ret = bmcv_image_vpp_convert_padding(pSophgoContext->m_bmContext->handle(), 1, image_aligned, &pSophgoContext->m_resized_imgs[i],
                                                        &padding_attr, &crop_rect);
#else
            auto ret = bmcv_image_vpp_convert(pSophgoContext->m_bmContext->handle(), 1 , image, &pSophgoContext->m_resized_imgs[i]);                            
#endif
            assert(BM_SUCCESS == ret);
#if DUMP_FILE
            static int b = 0;
            char szpath2[256] = {0}; 
            sprintf(szpath2,"resized%d.bmp",b);
            std::string strPath2(szpath2);
            bm_image_write_to_bmp(pSophgoContext->m_resized_imgs[i], strPath2.c_str());
            b++;
#endif
            if (need_copy) bm_image_destroy(image_aligned);
        }
        ret = bmcv_image_convert_to(pSophgoContext->m_bmContext->handle(), image_n, pSophgoContext->converto_attr, pSophgoContext->m_resized_imgs.data(), pSophgoContext->m_converto_imgs.data());
        CV_Assert(ret == 0);

        if(image_n != pSophgoContext->max_batch) image_n = pSophgoContext->m_bmNetwork->get_nearest_batch(image_n);
        bm_device_mem_t input_dev_mem;
        bm_image_get_contiguous_device_mem(image_n, pSophgoContext->m_converto_imgs.data(), &input_dev_mem);
        input_tensor->set_device_mem(&input_dev_mem);
        input_tensor->set_shape_by_dim(0, image_n);

        return common::ErrorCode::SUCCESS;
    }

    float UnetPre::get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *pIsAligWidth)
    {
    float ratio;
    float r_w = (float)dst_w / src_w;
    float r_h = (float)dst_h / src_h;
    if (r_h > r_w){
        *pIsAligWidth = true;
        ratio = r_w;
    }
    else{
        *pIsAligWidth = false;
        ratio = r_h;
    }
    return ratio;
    }
}
}
}