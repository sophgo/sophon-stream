#include "Yolov5Post.h"
#include "../context/SophgoContext.h"
#include "common/Logger.h"
#include "common/type_trans.hpp"

namespace sophon_stream
{
  namespace algorithm
  {
    namespace post_process
    {

      void Yolov5Post::init(algorithm::Context &context)
      {
      }

      int Yolov5Post::argmax(float *data, int num)
      {
        float max_value = 0.0;
        int max_index = 0;
        for (int i = 0; i < num; ++i)
        {
          float value = data[i];
          if (value > max_value)
          {
            max_value = value;
            max_index = i;
          }
        }

        return max_index;
      }

      float Yolov5Post::sigmoid(float x)
      {
        return 1.0 / (1 + expf(-x));
      }

      void Yolov5Post::NMS(YoloV5BoxVec &dets, float nmsConfidence)
      {
        int length = dets.size();
        int index = length - 1;

        std::sort(dets.begin(), dets.end(), [](const YoloV5Box &a, const YoloV5Box &b)
                  { return a.score < b.score; });

        std::vector<float> areas(length);
        for (int i = 0; i < length; i++)
        {
          areas[i] = dets[i].width * dets[i].height;
        }

        while (index > 0)
        {
          int i = 0;
          while (i < index)
          {
            float left = std::max(dets[index].x, dets[i].x);
            float top = std::max(dets[index].y, dets[i].y);
            float right = std::min(dets[index].x + dets[index].width, dets[i].x + dets[i].width);
            float bottom = std::min(dets[index].y + dets[index].height, dets[i].y + dets[i].height);
            float overlap = std::max(0.0f, right - left) * std::max(0.0f, bottom - top);
            if (overlap / (areas[index] + areas[i] - overlap) > nmsConfidence)
            {
              areas.erase(areas.begin() + i);
              dets.erase(dets.begin() + i);
              index--;
            }
            else
            {
              i++;
            }
          }
          index--;
        }
      }

      void Yolov5Post::initTpuKernel(algorithm::Context &context)
      {
        context::SophgoContext *pSophgoContext = dynamic_cast<context::SophgoContext *>(&context);
        pSophgoContext->has_init = true;

        tpu_kernel_module_t tpu_module;
        std::string tpu_kernel_module_path = "../../share/3rdparty/tpu_kernel_module/libbm1684x_kernel_module.so";
        tpu_module = tpu_kernel_load_module_file(pSophgoContext->m_bmContext->handle(), tpu_kernel_module_path.c_str());
        pSophgoContext->func_id = tpu_kernel_get_function(pSophgoContext->m_bmContext->handle(), tpu_module, "tpu_kernel_api_yolov5_detect_out");
        std::cout << "Using tpu_kernel yolo postprocession, kernel funtion id: " << pSophgoContext->func_id << std::endl;

        return;
      }

      void Yolov5Post::setTpuKernelMem(algorithm::Context &context, common::ObjectMetadatas &objectMetadatas)
      {
        context::SophgoContext *pSophgoContext = dynamic_cast<context::SophgoContext *>(&context);
        int out_len_max = 25200 * 7;
        int input_num = objectMetadatas[0]->mOutputBMtensors->tensors.size(); // 3
        int batch_num = 1;                                           // 4b has bug, now only for 1b.

        bm_handle_t handle_ = pSophgoContext->m_bmContext->handle();
        bm_device_mem_t in_dev_mem[input_num];
        for (int i = 0; i < input_num; i++)
          in_dev_mem[i] = objectMetadatas[0]->mOutputBMtensors->tensors[i]->device_mem;

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

      // tpu-kernel
      void Yolov5Post::postProcess(algorithm::Context &context, common::ObjectMetadatas &objectMetadatas)
      {
        if(objectMetadatas.size() == 0)
          return;
        if(objectMetadatas[0]->mFrame->mEndOfStream)
              return;
        context::SophgoContext *pSophgoContext = dynamic_cast<context::SophgoContext *>(&context);
        if(pSophgoContext->use_tpu_kernel)
        {
          // if(!pSophgoContext->has_init)
          // initTpuKernel(context);
          if(objectMetadatas.size() < pSophgoContext->max_batch)
          {
            IVS_DEBUG("objectMetadatas size too small !!!!!!!!!");
            return;
          }
          setTpuKernelMem(context, objectMetadatas);
          for (int i = 0; i < pSophgoContext->max_batch; i++)
          {
            // if (pSophgoContext->mEndOfStream)
            bm_image image = *objectMetadatas[i]->mFrame->mSpData;
            int tx1 = 0, ty1 = 0;
  #ifdef USE_ASPECT_RATIO
            bool isAlignWidth = false;
            float ratio = context::get_aspect_scaled_ratio(image.width, image.height, pSophgoContext->m_net_w, pSophgoContext->m_net_h, &isAlignWidth);
            if (isAlignWidth)
            {
              ty1 = (int)((pSophgoContext->m_net_h - (int)(image.height * ratio)) / 2);
            }
            else
            {
              tx1 = (int)((pSophgoContext->m_net_w - (int)(image.width * ratio)) / 2);
            }
  #endif

            tpu_kernel_launch(pSophgoContext->m_bmContext->handle(), pSophgoContext->func_id, &pSophgoContext->api[i], sizeof(pSophgoContext->api[i]));
            bm_thread_sync(pSophgoContext->m_bmContext->handle());
            bm_memcpy_d2s_partial_offset(pSophgoContext->m_bmContext->handle(), (void *)(pSophgoContext->detect_num + i), pSophgoContext->detect_num_mem[i], pSophgoContext->api[i].batch_num * sizeof(int32_t), 0);
            bm_memcpy_d2s_partial_offset(pSophgoContext->m_bmContext->handle(), (void *)pSophgoContext->output_tensor[i], pSophgoContext->out_dev_mem[i], pSophgoContext->detect_num[i] * 7 * sizeof(float), 0); // 25200*7

            for (int bid = 0; bid < pSophgoContext->detect_num[i]; bid++)
            {
              YoloV5Box temp_bbox;
              temp_bbox.class_id = *(pSophgoContext->output_tensor[i] + 7 * bid + 1);
              if (temp_bbox.class_id == -1)
              {
                continue;
              }
              temp_bbox.score = *(pSophgoContext->output_tensor[i] + 7 * bid + 2);
              float centerX = (*(pSophgoContext->output_tensor[i] + 7 * bid + 3) + 1 - tx1) / ratio - 1;
              float centerY = (*(pSophgoContext->output_tensor[i] + 7 * bid + 4) + 1 - ty1) / ratio - 1;
              temp_bbox.width = (*(pSophgoContext->output_tensor[i] + 7 * bid + 5) + 0.5) / ratio;
              temp_bbox.height = (*(pSophgoContext->output_tensor[i] + 7 * bid + 6) + 0.5) / ratio;

              temp_bbox.x = MAX(int(centerX - temp_bbox.width / 2), 0);
              temp_bbox.y = MAX(int(centerY - temp_bbox.height / 2), 0);

              std::shared_ptr<common::ObjectMetadata> spObjData = std::make_shared<common::ObjectMetadata>();
              spObjData->mDetectedObjectMetadata = std::make_shared<common::DetectedObjectMetadata>();
              spObjData->mDetectedObjectMetadata->mBox.mX = temp_bbox.x;
              spObjData->mDetectedObjectMetadata->mBox.mY = temp_bbox.y;
              spObjData->mDetectedObjectMetadata->mBox.mWidth = temp_bbox.width;
              spObjData->mDetectedObjectMetadata->mBox.mHeight = temp_bbox.height;
              spObjData->mDetectedObjectMetadata->mScores.push_back(temp_bbox.score);
              spObjData->mDetectedObjectMetadata->mClassify = temp_bbox.class_id;
              objectMetadatas[i]->mSubObjectMetadatas.push_back(spObjData);
            }
          }
          int input_num = pSophgoContext->m_bmNetwork->outputTensorNum();
          for(int i = 0;i < input_num;++i)
          {
            bm_free_device(*objectMetadatas[0]->mAlgorithmHandle, pSophgoContext->out_dev_mem[i]);
            bm_free_device(*objectMetadatas[0]->mAlgorithmHandle, pSophgoContext->detect_num_mem[i]);
          }
        }
        else
        {
          YoloV5BoxVec yolobox_vec;
          std::vector<std::shared_ptr<BMNNTensor>> outputTensors(pSophgoContext->output_num);
          for(int i=0; i<pSophgoContext->output_num; i++){
              // outputTensors[i] = objectMetadatas[0]->mOutputBMtensors->tensors[i];
              outputTensors[i] = std::make_shared<BMNNTensor>(objectMetadatas[0]->mOutputBMtensors->handle, pSophgoContext->m_bmNetwork->m_netinfo->output_names[i],
                pSophgoContext->m_bmNetwork->m_netinfo->output_scales[i], objectMetadatas[0]->mOutputBMtensors->tensors[i].get(), pSophgoContext->m_bmNetwork->is_soc);
          }

          for(int batch_idx = 0; batch_idx < pSophgoContext->max_batch; ++ batch_idx)
          {
              yolobox_vec.clear();
              if(pSophgoContext->mEndOfStream) continue;
              int frame_width = pSophgoContext->m_frame_w;
              int frame_height = pSophgoContext->m_frame_h;

              int tx1 = 0, ty1 = 0;
              #ifdef USE_ASPECT_RATIO
                  bool isAlignWidth = false;
                  float ratio = context::get_aspect_scaled_ratio(frame_width, frame_height,
                  pSophgoContext->m_net_w, pSophgoContext->m_net_h, &isAlignWidth);
                  if (isAlignWidth) {
                  ty1 = (int)((pSophgoContext->m_net_h - (int)(frame_height*ratio)) / 2);
                  }else{
                  tx1 = (int)((pSophgoContext->m_net_w - (int)(frame_width*ratio)) / 2);
                  }
              #endif

              int min_idx = 0;
              int box_num = 0;
              for(int i=0; i<pSophgoContext->output_num; i++){
                auto output_shape = pSophgoContext->m_bmNetwork->outputTensor(i)->get_shape();
                auto output_dims = output_shape->num_dims;
                assert(output_dims == 3 || output_dims == 5);
                if(output_dims == 5){
                    box_num += output_shape->dims[1] * output_shape->dims[2] * output_shape->dims[3];
                }

                if(pSophgoContext->min_dim>output_dims){
                    min_idx = i;
                    pSophgoContext->min_dim = output_dims;
                }
              }

              auto out_tensor = outputTensors[min_idx];
              int nout = out_tensor->get_shape()->dims[pSophgoContext->min_dim-1];
              pSophgoContext->m_class_num = nout - 5;

              float* output_data = nullptr;
              std::vector<float> decoded_data;

              if(pSophgoContext->min_dim ==3 && pSophgoContext->output_num !=1){
              std::cout<<"--> WARNING: the current bmodel has redundant outputs"<<std::endl;
              std::cout<<"             you can remove the redundant outputs to improve performance"<< std::endl;
              std::cout<<std::endl;
              }

              if(pSophgoContext->min_dim == 5){
                const std::vector<std::vector<std::vector<int>>> anchors{
                    {{10, 13}, {16, 30}, {33, 23}},
                    {{30, 61}, {62, 45}, {59, 119}},
                    {{116, 90}, {156, 198}, {373, 326}}};
                const int anchor_num = anchors[0].size();
                assert(pSophgoContext->output_num == (int)anchors.size());
                assert(box_num>0);
                if((int)decoded_data.size() != box_num*nout){
                    decoded_data.resize(box_num*nout);
                }
                float *dst = decoded_data.data();
                for(int tidx = 0; tidx < pSophgoContext->output_num; ++tidx) {
                    auto output_tensor = outputTensors[tidx];
                    int feat_c = output_tensor->get_shape()->dims[1];
                    int feat_h = output_tensor->get_shape()->dims[2];
                    int feat_w = output_tensor->get_shape()->dims[3];
                    int area = feat_h * feat_w;
                    assert(feat_c == anchor_num);
                    int feature_size = feat_h*feat_w*nout;
                    float *tensor_data = (float*)output_tensor->get_cpu_data() + batch_idx*feat_c*area*nout;
                    for (int anchor_idx = 0; anchor_idx < anchor_num; anchor_idx++)
                    {
                      float *ptr = tensor_data + anchor_idx*feature_size;
                      for (int i = 0; i < area; i++) {
                          dst[0] = (sigmoid(ptr[0]) * 2 - 0.5 + i % feat_w) / feat_w * pSophgoContext->m_net_w;
                          dst[1] = (sigmoid(ptr[1]) * 2 - 0.5 + i / feat_w) / feat_h * pSophgoContext->m_net_h;
                          dst[2] = pow((sigmoid(ptr[2]) * 2), 2) * anchors[tidx][anchor_idx][0];
                          dst[3] = pow((sigmoid(ptr[3]) * 2), 2) * anchors[tidx][anchor_idx][1];
                          dst[4] = sigmoid(ptr[4]);
                          float score = dst[4];
                          if (score > pSophgoContext->m_thresh[0]) {
                            for(int d=5; d<nout; d++){
                                dst[d] = sigmoid(ptr[d]);
                            }
                          }
                          dst += nout;
                          ptr += nout;
                      }
                    }
                }
                output_data = decoded_data.data();
              } else {
                assert(box_num == 0 || box_num == out_tensor->get_shape()->dims[1]);
                box_num = out_tensor->get_shape()->dims[1];
                output_data = (float*)out_tensor->get_cpu_data() + batch_idx*box_num*nout;
              }

              for (int i = 0; i < box_num; i++) {
                float* ptr = output_data+i*nout;
                float score = ptr[4];
                int class_id = argmax(&ptr[5], pSophgoContext->m_class_num);
                float confidence = ptr[class_id + 5];
                if (confidence * score > pSophgoContext->m_thresh[0])
                {
                    float centerX = (ptr[0]+1 - tx1)/ratio - 1;
                    float centerY = (ptr[1]+1 - ty1)/ratio - 1;
                    float width = (ptr[2]+0.5) / ratio;
                    float height = (ptr[3]+0.5) / ratio;

                    YoloV5Box box;
                    box.x = int(centerX - width / 2);
                    if (box.x < 0) box.x = 0;
                    box.y = int(centerY - height / 2);
                    if (box.y < 0) box.y = 0;
                    box.width = width;
                    box.height = height;
                    box.class_id = class_id;
                    box.score = confidence * score;
                    yolobox_vec.push_back(box);
                }
              }
              // printf("\n --> valid boxes number = %d\n", (int)yolobox_vec.size());
              #if USE_MULTICLASS_NMS
                  std::vector<YoloV5BoxVec> class_vec(m_class_num);
                  for (auto& box : yolobox_vec){
                  class_vec[box.class_id].push_back(box);
                  }
                  for (auto& cls_box : class_vec){
                  NMS(cls_box, m_nmsThreshold);
                  }
                  yolobox_vec.clear();
                  for (auto& cls_box : class_vec){
                  yolobox_vec.insert(yolobox_vec.end(), cls_box.begin(), cls_box.end());
                  }
              #else
                  NMS(yolobox_vec, pSophgoContext->m_thresh[1]);
              #endif

              for (auto bbox : yolobox_vec) {
                std::shared_ptr<common::ObjectMetadata> spObjData = std::make_shared<common::ObjectMetadata>();
                spObjData->mDetectedObjectMetadata = std::make_shared<common::DetectedObjectMetadata>();
                spObjData->mDetectedObjectMetadata->mBox.mX = bbox.x;
                spObjData->mDetectedObjectMetadata->mBox.mY = bbox.y;
                spObjData->mDetectedObjectMetadata->mBox.mWidth = bbox.width;
                spObjData->mDetectedObjectMetadata->mBox.mHeight = bbox.height;
                spObjData->mDetectedObjectMetadata->mScores.push_back(bbox.score);
                spObjData->mDetectedObjectMetadata->mClassify = bbox.class_id;
                objectMetadatas[batch_idx]->mSubObjectMetadatas.push_back(spObjData);
              }
          }
        }
      }

    

    } // namespace post_process
  }   // namespace algorithm
} // namespace sophon_stream