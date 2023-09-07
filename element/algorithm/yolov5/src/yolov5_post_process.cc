//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "yolov5_post_process.h"

#include <cmath>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

void Yolov5PostProcess::init(std::shared_ptr<Yolov5Context> context) {}

int Yolov5PostProcess::argmax(float* data, int num) {
  float max_value = 0.0;
  int max_index = 0;
  for (int i = 0; i < num; ++i) {
    float value = data[i];
    if (value > max_value) {
      max_value = value;
      max_index = i;
    }
  }

  return max_index;
}

float Yolov5PostProcess::sigmoid(float x) { return 1.0 / (1 + expf(-x)); }

float Yolov5PostProcess::get_aspect_scaled_ratio(int src_w, int src_h,
                                                 int dst_w, int dst_h,
                                                 bool* pIsAligWidth) {
  float ratio;
  float r_w = (float)dst_w / src_w;
  float r_h = (float)dst_h / src_h;
  if (r_h > r_w) {
    *pIsAligWidth = true;
    ratio = r_w;
  } else {
    *pIsAligWidth = false;
    ratio = r_h;
  }
  return ratio;
}

void Yolov5PostProcess::NMS(YoloV5BoxVec& dets, float nmsConfidence) {
  int length = dets.size();
  int index = length - 1;

  std::sort(
      dets.begin(), dets.end(),
      [](const YoloV5Box& a, const YoloV5Box& b) { return a.score < b.score; });

  std::vector<float> areas(length);
  for (int i = 0; i < length; i++) {
    areas[i] = dets[i].width * dets[i].height;
  }

  while (index > 0) {
    int i = 0;
    while (i < index) {
      float left = std::max(dets[index].x, dets[i].x);
      float top = std::max(dets[index].y, dets[i].y);
      float right = std::min(dets[index].x + dets[index].width,
                             dets[i].x + dets[i].width);
      float bottom = std::min(dets[index].y + dets[index].height,
                              dets[i].y + dets[i].height);
      float overlap =
          std::max(0.0f, right - left) * std::max(0.0f, bottom - top);
      if (overlap / (areas[index] + areas[i] - overlap) > nmsConfidence) {
        areas.erase(areas.begin() + i);
        dets.erase(dets.begin() + i);
        index--;
      } else {
        i++;
      }
    }
    index--;
  }
}

void Yolov5PostProcess::setTpuKernelMem(
    std::shared_ptr<Yolov5Context> context,
    common::ObjectMetadatas& objectMetadatas, tpu_kernel& tpu_k) {
  int out_len_max = 25200 * 7;
  if (objectMetadatas[0]->mFrame->mEndOfStream) return;
  int input_num = objectMetadatas[0]->mOutputBMtensors->tensors.size();  // 3
  int batch_num = 1;  // 4b has bug, now only for 1b.
  tpu_k.func_id = context->func_id;
  const std::vector<std::vector<std::vector<int>>> anchors{
      {{10, 13}, {16, 30}, {33, 23}},
      {{30, 61}, {62, 45}, {59, 119}},
      {{116, 90}, {156, 198}, {373, 326}}};
  bm_handle_t handle_ = context->bmContext->handle();

  std::vector<std::vector<std::shared_ptr<bm_device_mem_t>>> in_dev_mems(
      context->max_batch,
      std::vector<std::shared_ptr<bm_device_mem_t>>(input_num));
  for (int batch_idx = 0; batch_idx < context->max_batch; ++batch_idx) {
    if (objectMetadatas[batch_idx]->mFrame->mEndOfStream) break;
    for (int i = 0; i < input_num; i++)
      in_dev_mems[batch_idx][i] = std::make_shared<bm_device_mem_t>(
          objectMetadatas[batch_idx]->mOutputBMtensors->tensors[i]->device_mem);
    tpu_k.output_tensor[batch_idx] = new float[out_len_max];
    for (int j = 0; j < input_num; ++j) {
      tpu_k.api[batch_idx].bottom_addr[j] =
          bm_mem_get_device_addr(*in_dev_mems[batch_idx][j]);
    }
    auto ret = bm_malloc_device_byte(handle_, &tpu_k.out_dev_mem[batch_idx],
                                     out_len_max * sizeof(float));
    assert(BM_SUCCESS == ret);
    ret = bm_malloc_device_byte(handle_, &tpu_k.detect_num_mem[batch_idx],
                                batch_num * sizeof(int32_t));
    assert(BM_SUCCESS == ret);
    tpu_k.api[batch_idx].top_addr =
        bm_mem_get_device_addr(tpu_k.out_dev_mem[batch_idx]);
    tpu_k.api[batch_idx].detected_num_addr =
        bm_mem_get_device_addr(tpu_k.detect_num_mem[batch_idx]);

    // config
    tpu_k.api[batch_idx].input_num = input_num;
    tpu_k.api[batch_idx].batch_num = batch_num;
    for (int j = 0; j < input_num; ++j) {
      tpu_k.api[batch_idx].hw_shape[j][0] =
          context->bmNetwork->outputTensor(j)->get_shape()->dims[2];
      tpu_k.api[batch_idx].hw_shape[j][1] =
          context->bmNetwork->outputTensor(j)->get_shape()->dims[3];
    }
    tpu_k.api[batch_idx].num_classes = context->class_num;
    tpu_k.api[batch_idx].num_boxes = anchors[0].size();
    tpu_k.api[batch_idx].keep_top_k = 200;
    tpu_k.api[batch_idx].nms_threshold =
        0.1 > context->thresh_nms ? 0.1 : context->thresh_nms;
    tpu_k.api[batch_idx].confidence_threshold =
        0.1 > context->thresh_conf_min ? 0.1 : context->thresh_conf_min;
    auto it = tpu_k.api[batch_idx].bias;
    for (const auto& subvector2 : anchors) {
      for (const auto& subvector1 : subvector2) {
        it = copy(subvector1.begin(), subvector1.end(), it);
      }
    }
    for (int j = 0; j < input_num; ++j) {
      tpu_k.api[batch_idx].anchor_scale[j] =
          context->net_h /
          context->bmNetwork->outputTensor(j)->get_shape()->dims[2];
    }
    tpu_k.api[batch_idx].clip_box = 1;
  }
}

void Yolov5PostProcess::postProcess(std::shared_ptr<Yolov5Context> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  if (context->use_tpu_kernel) {
    postProcessTPUKERNEL(context, objectMetadatas);
  } else {
    postProcessCPU(context, objectMetadatas);
  }
}

void Yolov5PostProcess::postProcessTPUKERNEL(
    std::shared_ptr<Yolov5Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  tpu_kernel tpu_k;
  setTpuKernelMem(context, objectMetadatas, tpu_k);
  for (int i = 0; i < context->max_batch; i++) {
    if (objectMetadatas[i]->mFrame->mEndOfStream) break;
    bm_image image = *objectMetadatas[i]->mFrame->mSpData;
    int tx1 = 0, ty1 = 0;
#ifdef USE_ASPECT_RATIO
    bool isAlignWidth = false;
    float ratio =
        context->roi_predefined
            ? get_aspect_scaled_ratio(context->roi.crop_w, context->roi.crop_h,
                                      context->net_w, context->net_h,
                                      &isAlignWidth)
            : get_aspect_scaled_ratio(image.width, image.height, context->net_w,
                                      context->net_h, &isAlignWidth);
    if (isAlignWidth) {
      ty1 = (int)((context->net_h -
                   (int)((context->roi_predefined ? context->roi.crop_h
                                                  : image.height) *
                         ratio)) /
                  2);
    } else {
      tx1 = (int)((context->net_w -
                   (int)((context->roi_predefined ? context->roi.crop_w
                                                  : image.width) *
                         ratio)) /
                  2);
    }
#endif

    tpu_kernel_launch(context->bmContext->handle(), tpu_k.func_id,
                      &tpu_k.api[i], sizeof(tpu_k.api[i]));
    bm_thread_sync(context->bmContext->handle());
    bm_memcpy_d2s_partial_offset(
        context->bmContext->handle(), (void*)(tpu_k.detect_num + i),
        tpu_k.detect_num_mem[i], tpu_k.api[i].batch_num * sizeof(int32_t), 0);
    bm_memcpy_d2s_partial_offset(
        context->bmContext->handle(), (void*)tpu_k.output_tensor[i],
        tpu_k.out_dev_mem[i], tpu_k.detect_num[i] * 7 * sizeof(float),
        0);  // 25200*7

    for (int bid = 0; bid < tpu_k.detect_num[i]; bid++) {
      YoloV5Box temp_bbox;
      temp_bbox.class_id = *(tpu_k.output_tensor[i] + 7 * bid + 1);
      if (temp_bbox.class_id == -1) {
        continue;
      }
      temp_bbox.score = *(tpu_k.output_tensor[i] + 7 * bid + 2);
      if (context->roi_predefined &&
          (temp_bbox.score <
           context->thresh_conf[context->class_names[temp_bbox.class_id]]))
        continue;
      float centerX =
          (*(tpu_k.output_tensor[i] + 7 * bid + 3) + 1 - tx1) / ratio - 1;
      float centerY =
          (*(tpu_k.output_tensor[i] + 7 * bid + 4) + 1 - ty1) / ratio - 1;
      temp_bbox.width = (*(tpu_k.output_tensor[i] + 7 * bid + 5) + 0.5) / ratio;
      temp_bbox.height =
          (*(tpu_k.output_tensor[i] + 7 * bid + 6) + 0.5) / ratio;

      temp_bbox.x = std::max(int(centerX - temp_bbox.width / 2), 0);
      temp_bbox.y = std::max(int(centerY - temp_bbox.height / 2), 0);

      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = temp_bbox.x;
      detData->mBox.mY = temp_bbox.y;
      detData->mBox.mWidth = temp_bbox.width;
      detData->mBox.mHeight = temp_bbox.height;
      detData->mScores.push_back(temp_bbox.score);
      detData->mClassify = temp_bbox.class_id;
      if (context->roi_predefined) {
        detData->mBox.mX += context->roi.start_x;
        detData->mBox.mY += context->roi.start_y;
      }
      objectMetadatas[i]->mDetectedObjectMetadatas.push_back(detData);
    }
    delete[] tpu_k.output_tensor[i];
    bm_free_device(context->bmContext->handle(), tpu_k.out_dev_mem[i]);
    bm_free_device(context->bmContext->handle(), tpu_k.detect_num_mem[i]);
  }
}

void Yolov5PostProcess::postProcessCPU(
    std::shared_ptr<Yolov5Context> context,
    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;
  YoloV5BoxVec yolobox_vec;
  int idx = 0;
  for (auto obj : objectMetadatas) {
    if (obj->mFrame->mEndOfStream) break;
    std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
    for (int i = 0; i < context->output_num; i++) {
      outputTensors[i] = std::make_shared<BMNNTensor>(
          obj->mOutputBMtensors->handle,
          context->bmNetwork->m_netinfo->output_names[i],
          context->bmNetwork->m_netinfo->output_scales[i],
          obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
    }

    yolobox_vec.clear();
    int frame_width = obj->mFrame->mWidth;
    int frame_height = obj->mFrame->mHeight;

    int tx1 = 0, ty1 = 0;
#ifdef USE_ASPECT_RATIO
    bool isAlignWidth = false;
    float ratio =
        context->roi_predefined
            ? get_aspect_scaled_ratio(context->roi.crop_w, context->roi.crop_h,
                                      context->net_w, context->net_h,
                                      &isAlignWidth)
            : get_aspect_scaled_ratio(frame_width, frame_height, context->net_w,
                                      context->net_h, &isAlignWidth);
    if (isAlignWidth) {
      ty1 = (int)((context->net_h -
                   (int)((context->roi_predefined ? context->roi.crop_h
                                                  : frame_height) *
                         ratio)) /
                  2);
    } else {
      tx1 = (int)((context->net_w -
                   (int)((context->roi_predefined ? context->roi.crop_w
                                                  : frame_width) *
                         ratio)) /
                  2);
    }
#endif
    int min_idx = 0;
    int box_num = 0;
    for (int i = 0; i < context->output_num; ++i) {
      auto output_shape = context->bmNetwork->outputTensor(i)->get_shape();
      auto output_dims = output_shape->num_dims;
      assert(output_dims == 3 || output_dims == 5);
      if (output_dims == 5) {
        box_num += output_shape->dims[1] * output_shape->dims[2] *
                   output_shape->dims[3];
      }

      if (context->min_dim > output_dims) {
        min_idx = i;
        context->min_dim = output_dims;
      }
    }

    auto out_tensor = outputTensors[min_idx];
    int nout = out_tensor->get_shape()->dims[context->min_dim - 1];

    float* output_data = nullptr;
    std::vector<float> decoded_data;

    if (context->min_dim == 3 && context->output_num != 1) {
      std::cout << "--> WARNING: the current bmodel has redundant outputs"
                << std::endl;
      std::cout << "             you can remove the redundant outputs to "
                   "improve performance"
                << std::endl;
      std::cout << std::endl;
    }
    if (context->min_dim == 5) {
      const std::vector<std::vector<std::vector<int>>> anchors{
          {{10, 13}, {16, 30}, {33, 23}},
          {{30, 61}, {62, 45}, {59, 119}},
          {{116, 90}, {156, 198}, {373, 326}}};
      const int anchor_num = anchors[0].size();
      assert(context->output_num == (int)anchors.size());
      assert(box_num > 0);
      if ((int)decoded_data.size() != box_num * nout) {
        decoded_data.resize(box_num * nout);
      }
      float* dst = decoded_data.data();
      for (int tidx = 0; tidx < context->output_num; ++tidx) {
        auto output_tensor = outputTensors[tidx];
        int feat_c = output_tensor->get_shape()->dims[1];
        int feat_h = output_tensor->get_shape()->dims[2];
        int feat_w = output_tensor->get_shape()->dims[3];
        int area = feat_h * feat_w;
        assert(feat_c == anchor_num);
        int feature_size = feat_h * feat_w * nout;
        float* tensor_data = (float*)output_tensor->get_cpu_data();

        for (int anchor_idx = 0; anchor_idx < anchor_num; anchor_idx++) {
          float* ptr = tensor_data + anchor_idx * feature_size;
          for (int i = 0; i < area; i++) {
            float score = sigmoid(ptr[4]);
            if (score >= context->thresh_conf_min) {
              int class_id = argmax(&ptr[5], context->class_num);
              float confidence = sigmoid(ptr[class_id + 5]);
              float cur_class_thresh =
                  context->class_thresh_valid
                      ? context->thresh_conf[context->class_names[class_id]]
                      : context->thresh_conf_min;
              if (confidence * score >= cur_class_thresh) {
                dst[0] = (sigmoid(ptr[0]) * 2 - 0.5 + i % feat_w) / feat_w *
                         context->net_w;
                dst[1] = (sigmoid(ptr[1]) * 2 - 0.5 + i / feat_w) / feat_h *
                         context->net_h;
                dst[2] = pow((sigmoid(ptr[2]) * 2), 2) *
                         anchors[tidx][anchor_idx][0];
                dst[3] = pow((sigmoid(ptr[3]) * 2), 2) *
                         anchors[tidx][anchor_idx][1];
                for (int d = 5; d < nout; d++) {
                  dst[d] = sigmoid(ptr[d]);
                }
                float centerX = (dst[0] + 1 - tx1) / ratio - 1;
                float centerY = (dst[1] + 1 - ty1) / ratio - 1;
                float width = (dst[2] + 0.5) / ratio;
                float height = (dst[3] + 0.5) / ratio;

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
            dst += nout;
            ptr += nout;
          }
        }
      }
      output_data = decoded_data.data();
    } else {
      assert(box_num == 0 || box_num == out_tensor->get_shape()->dims[1]);
      box_num = out_tensor->get_shape()->dims[1];
      output_data = (float*)out_tensor->get_cpu_data();
      for (int i = 0; i < box_num; i++) {
        float* ptr = output_data + i * nout;
        float score = ptr[4];
        int class_id = argmax(&ptr[5], context->class_num);
        float confidence = ptr[class_id + 5];
        if (confidence * score >
            (context->class_thresh_valid
                 ? context->thresh_conf[context->class_names[class_id]]
                 : context->thresh_conf_min)) {
          float centerX = (ptr[0] + 1 - tx1) / ratio - 1;
          float centerY = (ptr[1] + 1 - ty1) / ratio - 1;
          float width = (ptr[2] + 0.5) / ratio;
          float height = (ptr[3] + 0.5) / ratio;

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
    }

    NMS(yolobox_vec, context->thresh_nms);

    for (auto bbox : yolobox_vec) {
      std::shared_ptr<common::DetectedObjectMetadata> detData =
          std::make_shared<common::DetectedObjectMetadata>();
      detData->mBox.mX = bbox.x;
      detData->mBox.mY = bbox.y;
      detData->mBox.mWidth = bbox.width;
      detData->mBox.mHeight = bbox.height;
      detData->mScores.push_back(bbox.score);
      detData->mClassify = bbox.class_id;

      if (context->roi_predefined) {
        detData->mBox.mX += context->roi.start_x;
        detData->mBox.mY += context->roi.start_y;
      }

      obj->mDetectedObjectMetadatas.push_back(detData);
    }
    ++idx;
  }
}

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream