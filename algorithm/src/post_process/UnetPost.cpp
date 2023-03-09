#include "UnetPost.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"

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


common::Frame UnetPost::bm_image2Frame(bm_image & img)
{
    common::Frame f;

    f.mWidth = img.width;
    f.mHeight = img.height;
    f.mDataType = common::data_bmcv2stream(img.data_type);
    f.mFormatType = common::format_bmcv2stream(img.image_format);
    f.mChannel = 1;
    f.mDataSize = img.width * img.height * sizeof(uchar);
    void * buffers;
    bm_image_copy_device_to_host(img, &buffers);
    f.mData = std::make_shared<void>(buffers);

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
            std::shared_ptr<common::Frame> image_ptr = objectMetadatas[batch_idx]->mTransformFrame;
            int width = image_ptr->mWidth;
            int height = image_ptr->mHeight;
            std::shared_ptr<void> data = image_ptr->mData;
            // 转格式
            sophon_stream::common::FormatType format_type_stream = image_ptr->mFormatType;
            sophon_stream::common::DataType data_type_stream = image_ptr->mDataType;
            bm_image_format_ext format_type_bmcv = common::format_stream2bmcv(format_type_stream);
            bm_image_data_format_ext data_type_bmcv = common::data_stream2bmcv(data_type_stream);
            // 转成bm_image
            bm_image image;
            bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, format_type_bmcv, data_type_bmcv, &image);
            void* pt = data.get();
            bm_image_copy_host_to_device(image, (void**)&pt);

            bm_image result;
            // 取出一个输出tensor
            auto output_shape = outputTensor->get_shape();
            int feat_c = outputTensor->get_shape()->dims[1];
            int feat_h = outputTensor->get_shape()->dims[2];
            int feat_w = outputTensor->get_shape()->dims[3];
            int feature_size = feat_w * feat_c;
            std::shared_ptr<uchar> decoded_data(new uchar[feature_size],[](uchar * p){delete[] p;});
            std::shared_ptr<float> tensor_data(outputTensor->get_cpu_data() + batch_idx * feat_c * feature_size);
            // tensor --> bm_img
            if (feat_c == 1)
            {
                for(int i = 0;i<feature_size;++i)
                    decoded_data.get()[i] = sigmoid(tensor_data.get()[i]) > outThresh ? 255 : 0;
            }
            else
            {
                for(int i = 0;i<feature_size;++i)
                {
                    // for num_classes > 2, argmax should be utilized
                    decoded_data.get()[i] = (tensor_data.get()[i] > tensor_data.get()[i+feature_size]) ? 0 : 255;
                }
            }
            bm_image_create(pSophgoContext->m_bmContext->handle(), feat_h, feat_w, FORMAT_GRAY, image.data_type, &result);
            uchar* p = decoded_data.get();
            bm_image_copy_host_to_device(result, (void**)(&p));
            
            bm_image result_resized;
            auto ret = bm_image_create(pSophgoContext->m_bmContext->handle(), height, width, FORMAT_GRAY, image.data_type, &result_resized);
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
            auto Metadata = objectMetadatas[batch_idx]->mSegmentedObjectMetadatas;
            common::SegmentedObjectMetadata SegData;
            SegData.mFrame = std::make_shared<common::Frame>(bm_image2Frame(result_resized));
            Metadata.push_back(std::make_shared<common::SegmentedObjectMetadata>(SegData));

            bm_image_destroy(result);
        }
        return;
    }
}
}
}