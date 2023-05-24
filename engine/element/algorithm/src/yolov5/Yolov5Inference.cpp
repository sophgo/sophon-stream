#include "Yolov5Inference.h"
#include <fstream>

namespace sophon_stream
{
  namespace algorithm
  {
    namespace yolov5
    {

      Yolov5Inference::~Yolov5Inference()
      {
      }

      /**
       * init device and engine
       * @param[in] @param[in] context: model path,inputs and outputs name...
       */
      common::ErrorCode Yolov5Inference::init(Yolov5SophgoContext &context)
      {
        Yolov5SophgoContext *pSophgoContext = &context;
        float confThresh;
        float nmsThresh;
        std::string coco_names_file;
        pSophgoContext->m_thresh = context.threthold; // thresh[0] confThresh, thresh[1] nmsThresh
        pSophgoContext->m_class_num = context.numClass;

        // 1. get network
        BMNNHandlePtr handle = std::make_shared<BMNNHandle>(pSophgoContext->deviceId);
        pSophgoContext->m_bmContext = std::make_shared<BMNNContext>(handle, pSophgoContext->modelPath[0].c_str());
        pSophgoContext->m_bmNetwork = pSophgoContext->m_bmContext->network(0);

        pSophgoContext->handle = handle->handle();

        // 2. get input
        pSophgoContext->max_batch = pSophgoContext->m_bmNetwork->maxBatch();
        auto tensor = pSophgoContext->m_bmNetwork->inputTensor(0);
        pSophgoContext->input_num = pSophgoContext->m_bmNetwork->m_netinfo->input_num;
        pSophgoContext->m_net_channel = tensor->get_shape()->dims[1];
        pSophgoContext->m_net_h = tensor->get_shape()->dims[2];
        pSophgoContext->m_net_w = tensor->get_shape()->dims[3];

        // 3. get output
        pSophgoContext->output_num = pSophgoContext->m_bmNetwork->outputTensorNum();
        assert(pSophgoContext->output_num > 0);
        pSophgoContext->min_dim = pSophgoContext->m_bmNetwork->outputTensor(0)->get_shape()->num_dims;

        // 4. initialize bmimages
        pSophgoContext->m_resized_imgs.resize(pSophgoContext->max_batch);
        pSophgoContext->m_converto_imgs.resize(pSophgoContext->max_batch);
        // some API only accept bm_image whose stride is aligned to 64
        int aligned_net_w = FFALIGN(pSophgoContext->m_net_w, 64);
        int strides[3] = {aligned_net_w, aligned_net_w, aligned_net_w};
        for (int i = 0; i < pSophgoContext->max_batch; i++)
        {
          auto ret = bm_image_create(pSophgoContext->m_bmContext->handle(), pSophgoContext->m_net_h,
                                     pSophgoContext->m_net_w,
                                     FORMAT_RGB_PLANAR, DATA_TYPE_EXT_1N_BYTE, &pSophgoContext->m_resized_imgs[i], strides);
          assert(BM_SUCCESS == ret);
        }
        bm_image_alloc_contiguous_mem(pSophgoContext->max_batch, pSophgoContext->m_resized_imgs.data());
        bm_image_data_format_ext img_dtype = DATA_TYPE_EXT_FLOAT32;
        if (tensor->get_dtype() == BM_INT8)
        {
          img_dtype = DATA_TYPE_EXT_1N_BYTE_SIGNED;
        }

        // 5.converto
        float input_scale = tensor->get_scale();
        input_scale = input_scale * 1.0 / 255.f;
        pSophgoContext->converto_attr.alpha_0 = input_scale;
        pSophgoContext->converto_attr.beta_0 = 0;
        pSophgoContext->converto_attr.alpha_1 = input_scale;
        pSophgoContext->converto_attr.beta_1 = 0;
        pSophgoContext->converto_attr.alpha_2 = input_scale;
        pSophgoContext->converto_attr.beta_2 = 0;

        // 6. tpu_kernel postprocess
        if(pSophgoContext->use_tpu_kernel)
        {
          tpu_kernel_module_t tpu_module;
          std::string tpu_kernel_module_path = "../../share/3rdparty/tpu_kernel_module/libbm1684x_kernel_module.so";
          tpu_module = tpu_kernel_load_module_file(pSophgoContext->m_bmContext->handle(), tpu_kernel_module_path.c_str());
          pSophgoContext->func_id = tpu_kernel_get_function(pSophgoContext->m_bmContext->handle(), tpu_module, "tpu_kernel_api_yolov5_detect_out");
          std::cout << "Using tpu_kernel yolo postprocession, kernel funtion id: " << pSophgoContext->func_id << std::endl;

          int out_len_max = 25200 * 7;
          int input_num = pSophgoContext->m_bmNetwork->outputTensorNum();
          int batch_num = 1; // 4b has bug, now only for 1b.

          bm_handle_t handle_ = pSophgoContext->m_bmContext->handle();
          bm_device_mem_t in_dev_mem[input_num];
          for (int i = 0; i < input_num; i++)
            in_dev_mem[i] = *pSophgoContext->m_bmNetwork->outputTensor(i)->get_device_mem();

          for (int i = 0; i < pSophgoContext->max_batch; i++)
          {
            pSophgoContext->output_tensor[i] = new float[out_len_max];
            for (int j = 0; j < input_num; j++)
            {
              pSophgoContext->api[i].bottom_addr[j] = bm_mem_get_device_addr(in_dev_mem[j]) + i * in_dev_mem[j].size / pSophgoContext->max_batch;
            }
            auto ret = bm_malloc_device_byte(handle_, &pSophgoContext->out_dev_mem[i], out_len_max * sizeof(float));
            assert(BM_SUCCESS == ret);
            ret = bm_malloc_device_byte(handle_, &pSophgoContext->detect_num_mem[i], batch_num * sizeof(int32_t));
            assert(BM_SUCCESS == ret);
            pSophgoContext->api[i].top_addr = bm_mem_get_device_addr(pSophgoContext->out_dev_mem[i]);
            pSophgoContext->api[i].detected_num_addr = bm_mem_get_device_addr(pSophgoContext->detect_num_mem[i]);

            // config
            pSophgoContext->api[i].input_num = input_num;
            pSophgoContext->api[i].batch_num = batch_num;
            for (int j = 0; j < input_num; ++j)
            {
              pSophgoContext->api[i].hw_shape[j][0] = pSophgoContext->m_bmNetwork->outputTensor(j)->get_shape()->dims[2];
              pSophgoContext->api[i].hw_shape[j][1] = pSophgoContext->m_bmNetwork->outputTensor(j)->get_shape()->dims[3];
            }
            pSophgoContext->api[i].num_classes = pSophgoContext->m_class_num;
            const std::vector<std::vector<std::vector<int>>> anchors{
                {{10, 13}, {16, 30}, {33, 23}}, {{30, 61}, {62, 45}, {59, 119}}, {{116, 90}, {156, 198}, {373, 326}}};
            pSophgoContext->api[i].num_boxes = anchors[0].size();
            pSophgoContext->api[i].keep_top_k = 200;
            pSophgoContext->api[i].nms_threshold = MAX(0.1, pSophgoContext->m_thresh[1]);
            pSophgoContext->api[i].confidence_threshold = MAX(0.1, pSophgoContext->m_thresh[0]);
            auto it = pSophgoContext->api[i].bias;
            for (const auto &subvector2 : anchors)
            {
              for (const auto &subvector1 : subvector2)
              {
                it = copy(subvector1.begin(), subvector1.end(), it);
              }
            }
            for (int j = 0; j < input_num; j++)
              pSophgoContext->api[i].anchor_scale[j] = pSophgoContext->m_net_h / pSophgoContext->m_bmNetwork->outputTensor(j)->get_shape()->dims[2];
            pSophgoContext->api[i].clip_box = 1;

          }
        }
        
          return common::ErrorCode::SUCCESS;
      }

      /**
       * network predict output
       * @param[in] context: inputData and outputDat
       */
      common::ErrorCode Yolov5Inference::predict(Yolov5SophgoContext& context, common::ObjectMetadatas &objectMetadatas)
      {
        Yolov5SophgoContext *pSophgoContext = &context;
        
        if(objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

        int ret = 0;
        // if (!pSophgoContext->mEndOfStream)
        if(!objectMetadatas[0]->mFrame->mEndOfStream)
          ret = pSophgoContext->m_bmNetwork->forward(objectMetadatas[0]->mInputBMtensors->tensors, objectMetadatas[0]->mOutputBMtensors->tensors);
        return static_cast<common::ErrorCode>(ret);
      }

      void Yolov5Inference::uninit()
      {
      }

    } // namespace inference
  }   // namespace algorithm
} // namespace sophon_stream
