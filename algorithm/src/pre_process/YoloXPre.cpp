#include "YoloXPre.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"
#include "common/Clocker.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

#define DUMP_FILE 0

common::ErrorCode YoloXPre::preProcess(algorithm::Context& context,
    common::ObjectMetadatas& objectMetadatas)
{
    Clocker clocker;
    context::SophgoContext* pSophgoContext = dynamic_cast<context::SophgoContext*>(&context);
    std::vector<bm_image> images;
    for(auto& objMetadata:objectMetadatas)
    {
        if(objMetadata->mFrame->mEndOfStream)
        {
            // end of file, create a blank bmimg
            pSophgoContext->mEndOfStream = objMetadata->mFrame->mEndOfStream;
            auto tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
            pSophgoContext->m_frame_h = tensor->get_shape()->dims[2];
            pSophgoContext->m_frame_w = tensor->get_shape()->dims[3];
            objMetadata->mFrame->mSpData.reset(new bm_image,[&](bm_image* p){
            bm_image_destroy(*p);delete p;p=nullptr;});
            bm_image_create(pSophgoContext->m_bmContext->handle(), pSophgoContext->m_frame_h, pSophgoContext->m_frame_w, 
                FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, objMetadata->mFrame->mSpData.get());
            bm_image_alloc_dev_mem(*objMetadata->mFrame->mSpData);
        }
        else
        {
            pSophgoContext->m_frame_w = objMetadata->mFrame->mWidth;
            pSophgoContext->m_frame_h = objMetadata->mFrame->mHeight;
        }
        images.push_back(*objMetadata->mFrame->mSpData);
    }
    std::shared_ptr<BMNNTensor> input_tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
    int image_n = images.size();

    int ret = 0;
    for(int i = 0; i < image_n; ++i) {
        bm_image image0 = images[i];
        bm_image image1;
        // convert to RGB_PLANAR
        if(image0.image_format != FORMAT_BGR_PLANAR){
            bm_image_create(pSophgoContext->m_bmContext->handle(), image0.height, image0.width,
                FORMAT_BGR_PLANAR, image0.data_type, &image1);
                bm_image_alloc_dev_mem(image1, BMCV_IMAGE_FOR_IN);
                bmcv_image_storage_convert(pSophgoContext->m_bmContext->handle(),1,&image0,&image1);
        }
        else{
            image1 = image0;
        }

        bm_image image_aligned;
        bool need_copy = image1.width & (64-1);
        if(need_copy){
            int stride1[3], stride2[3];
            bm_image_get_stride(image1, stride1);
            stride2[0] = FFALIGN(stride1[0], 64);
            stride2[1] = FFALIGN(stride1[1], 64);
            stride2[2] = FFALIGN(stride1[2], 64);
            bm_image_create(pSophgoContext->m_bmContext->handle(), image1.height, image1.width,
                image1.image_format, image1.data_type, &image_aligned, stride2);

            bm_image_alloc_dev_mem(image_aligned, BMCV_IMAGE_FOR_IN);
            bmcv_copy_to_atrr_t copyToAttr;
            memset(&copyToAttr, 0, sizeof(copyToAttr));
            copyToAttr.start_x = 0;
            copyToAttr.start_y = 0;
            copyToAttr.if_padding = 1;
            bmcv_image_copy_to(pSophgoContext->m_bmContext->handle(), copyToAttr, image1, image_aligned);
            } else {
            image_aligned = image1;
        }

        float scale_w = float(pSophgoContext->m_net_w) / image_aligned.width;
        float scale_h = float(pSophgoContext->m_net_h) / image_aligned.height;

        int pad_w = pSophgoContext->m_net_w;
        int pad_h = pSophgoContext->m_net_h;

        float scale_min = scale_h;
        if (scale_w < scale_h) {
            pad_h = image_aligned.height*scale_w;
            scale_min = scale_w;
        } else {
            pad_w = image_aligned.width*scale_h;
        }
        bmcv_padding_atrr_t padding_attr;
        memset(&padding_attr, 0, sizeof(padding_attr));
        padding_attr.dst_crop_stx = 0;
        padding_attr.dst_crop_sty = 0;
        padding_attr.dst_crop_w = pad_w;
        padding_attr.dst_crop_h = pad_h;
        padding_attr.padding_b = 114;
        padding_attr.padding_g = 114;
        padding_attr.padding_r = 114;

        bmcv_rect_t crop_rect {0, 0, image1.width, image1.height};
        auto ret = bmcv_image_vpp_convert_padding(pSophgoContext->m_bmContext->handle(), 1, image_aligned, 
            &pSophgoContext->m_resized_imgs[i],
                &padding_attr, &crop_rect);
        assert(BM_SUCCESS == ret);
#if DUMP_FILE
        // static int b = 0;
        // char szpath2[256] = {0}; 
        // sprintf(szpath2,"resized.bmp",b);
        // std::string strPath2(szpath2);
        bm_image_write_to_bmp(pSophgoContext->m_resized_imgs[i], "resized.bmp");
        // b++;
#endif
        if(image0.image_format != FORMAT_BGR_PLANAR){
            bm_image_destroy(image1);
        }
        if(need_copy) bm_image_destroy(image_aligned);
    }

    ret = bmcv_image_convert_to(pSophgoContext->m_bmContext->handle(), image_n, 
    pSophgoContext->converto_attr, pSophgoContext->m_resized_imgs.data(), pSophgoContext->m_converto_imgs.data());
    CV_Assert(ret == 0);

    //3. attach to tensor
    if(image_n != pSophgoContext->max_batch) image_n = pSophgoContext->m_bmNetwork->get_nearest_batch(image_n); 
    bm_device_mem_t input_dev_mem;
    bm_image_get_contiguous_device_mem(image_n, pSophgoContext->m_converto_imgs.data(), &input_dev_mem);
    input_tensor->set_device_mem(&input_dev_mem);
    input_tensor->set_shape_by_dim(0, image_n);  // set real batch number
    std::cout<<"yoloX pre cost: "<<clocker.tell_us()<<std::endl;
    
    return common::ErrorCode::SUCCESS;
}



}
}
}