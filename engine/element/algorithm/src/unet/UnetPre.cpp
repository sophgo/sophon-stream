#include "UnetPre.h"
#include "UnetSophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"
#include "common/ObjectMetadata.h"


namespace sophon_stream {
namespace algorithm {
namespace unet {

#define DUMP_FILE 0
#define USE_ASPECT_RATIO 1

void UnetPre::initTensors(UnetSophgoContext &context, common::ObjectMetadatas &objectMetadatas)
{
    /**
     * 若要将前后处理单独放在一个element上，需要保证前后处理和推理使用的tpu memory能对应起来
     * 在preprocess阶段初始化objectMetadatas[0]的BMtensor和handle(handle实际上不必要，只要在同一张卡上，handle可以通用)
     * 推理阶段更新这块memory
     * 处理阶段将这块memory取出，配置内存或解码
     * objectMetadata使用完之后销毁
     */
    UnetSophgoContext *pSophgoContext = &context;
    
    objectMetadatas[0]->mInputBMtensors = std::make_shared<sophon_stream::common::bmTensors>();
    objectMetadatas[0]->mOutputBMtensors = std::make_shared<sophon_stream::common::bmTensors>();

    objectMetadatas[0]->mInputBMtensors.reset(new sophon_stream::common::bmTensors(), [](sophon_stream::common::bmTensors *p){
        for(int i = 0;i < p->tensors.size();++i)
            bm_free_device(p->handle, p->tensors[i]->device_mem);
        delete p;
        p = nullptr; 
    });
    objectMetadatas[0]->mOutputBMtensors.reset(new sophon_stream::common::bmTensors(), [](sophon_stream::common::bmTensors *p){
        for (int i = 0; i < p->tensors.size(); ++i)
            bm_free_device(p->handle, p->tensors[i]->device_mem);
        delete p;
        p = nullptr;
    });
    objectMetadatas[0]->mInputBMtensors->handle = pSophgoContext->handle;
    objectMetadatas[0]->mOutputBMtensors->handle = pSophgoContext->handle;

    objectMetadatas[0]->mInputBMtensors->tensors.resize(pSophgoContext->input_num);
    objectMetadatas[0]->mOutputBMtensors->tensors.resize(pSophgoContext->output_num);
    for (int i = 0; i < pSophgoContext->input_num; ++i)
    {
        objectMetadatas[0]->mInputBMtensors->tensors[i] = std::make_shared<bm_tensor_t>();
        objectMetadatas[0]->mInputBMtensors->tensors[i]->dtype = pSophgoContext->m_bmNetwork->m_netinfo->input_dtypes[i];
        objectMetadatas[0]->mInputBMtensors->tensors[i]->shape = pSophgoContext->m_bmNetwork->m_netinfo->stages[0].input_shapes[i];
        objectMetadatas[0]->mInputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
        // 前处理的mInpuptBMtensors不需要申请内存，在preprocess中通过std::move得到
        int input_bytes = pSophgoContext->max_batch * pSophgoContext->m_net_channel * pSophgoContext->m_net_h * pSophgoContext->m_net_w;
        if (BM_FLOAT32 == pSophgoContext->m_bmNetwork->m_netinfo->input_dtypes[0]);
            input_bytes *= 4;
        // assert(BM_SUCCESS == ret);
    }

    for (int i = 0; i < pSophgoContext->output_num; ++i)
    {
        objectMetadatas[0]->mOutputBMtensors->tensors[i] = std::make_shared<bm_tensor_t>();
        objectMetadatas[0]->mOutputBMtensors->tensors[i]->dtype = pSophgoContext->m_bmNetwork->m_netinfo->output_dtypes[i];
        objectMetadatas[0]->mOutputBMtensors->tensors[i]->shape = pSophgoContext->m_bmNetwork->m_netinfo->stages[0].output_shapes[i];
        objectMetadatas[0]->mOutputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
        size_t max_size = 0;
        // 后处理的mOutputBMtensor需要申请内存，在forward中更新
        for (int s = 0; s < pSophgoContext->m_bmNetwork->m_netinfo->stage_num; s++)
        {
            size_t out_size = bmrt_shape_count(&pSophgoContext->m_bmNetwork->m_netinfo->stages[s].output_shapes[i]);
            if (max_size < out_size)
            {
                max_size = out_size;
            }
        }
        if (BM_FLOAT32 == pSophgoContext->m_bmNetwork->m_netinfo->output_dtypes[i])
            max_size *= 4;
        auto ret = bm_malloc_device_byte(objectMetadatas[0]->mOutputBMtensors->handle, &objectMetadatas[0]->mOutputBMtensors->tensors[i]->device_mem,
                                            max_size);
        assert(BM_SUCCESS == ret);
    }
}

common::ErrorCode UnetPre::preProcess(UnetSophgoContext& context,
    common::ObjectMetadatas & objectMetadatas){
        if(objectMetadatas.size() == 0)
            return common::ErrorCode::SUCCESS;
        UnetSophgoContext* pSophgoContext = &context;
        int image_n = objectMetadatas.size();
        std::shared_ptr<BMNNTensor> input_tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
        bmcv_resize_image resize_attr;
        int ret = 0;

        initTensors(context, objectMetadatas);

        for(int i = 0;i < image_n; ++i)
        {
            // 从objectMetadata读取frame信息
            if(objectMetadatas[i]->mFrame->mEndOfStream)
                return common::ErrorCode::SUCCESS;
            pSophgoContext->m_frame_w = objectMetadatas[i]->mFrame->mWidth;
            pSophgoContext->m_frame_h = objectMetadatas[i]->mFrame->mHeight;
            int width = objectMetadatas[i]->mFrame->mWidth;
            int height = objectMetadatas[i]->mFrame->mHeight;

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

        // malloc m_converto_imgs
        bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
        auto tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
        if (tensor->get_dtype() == BM_INT8)
        {
            img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
        }
        ret = bm_image_create_batch(pSophgoContext->m_bmContext->handle(), pSophgoContext->m_net_h,
                                        pSophgoContext->m_net_w, FORMAT_RGB_PLANAR, img_dtype, pSophgoContext->m_converto_imgs.data(), pSophgoContext->max_batch);
        assert(BM_SUCCESS == ret);

        // converto
        ret = bmcv_image_convert_to(pSophgoContext->m_bmContext->handle(), image_n, pSophgoContext->converto_attr, pSophgoContext->m_resized_imgs.data(), pSophgoContext->m_converto_imgs.data());
        CV_Assert(ret == 0);

        if(image_n != pSophgoContext->max_batch) 
            image_n = pSophgoContext->m_bmNetwork->get_nearest_batch(image_n);
        bm_device_mem_t input_dev_mem;
        bm_image_get_contiguous_device_mem(image_n, pSophgoContext->m_converto_imgs.data(), &input_dev_mem);
        
        // set inputBMtensors with std::move
        objectMetadatas[0]->mInputBMtensors->tensors[0]->device_mem = std::move(input_dev_mem);

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