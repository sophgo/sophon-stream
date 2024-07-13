//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_CONTEXT_H_

#include "algorithmApi/context.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

#define USE_ASPECT_RATIO 1
#define USE_MULTICLASS_NMS 0

#define MAX_YOLO_INPUT_NUM 8
#define MAX_YOLO_ANCHOR_NUM 8
typedef struct {
  unsigned long long bottom_addr[MAX_YOLO_INPUT_NUM];
  unsigned long long top_addr;
  unsigned long long detected_num_addr;
  int input_num;
  int batch_num;
  int hw_shape[MAX_YOLO_INPUT_NUM][2];
  int num_classes;
  int num_boxes;
  int keep_top_k;
  float nms_threshold;
  float confidence_threshold;
  float bias[MAX_YOLO_INPUT_NUM * MAX_YOLO_ANCHOR_NUM * 2];
  float anchor_scale[MAX_YOLO_INPUT_NUM];
  int clip_box;
  int agnostic_nms;
} tpu_kernel_api_yolov5NMS_t;

#define MAX_BATCH 16

class Yolov5Context : public ::sophon_stream::element::Context {
 public:
  int deviceId;  // 设备ID

  std::shared_ptr<BMNNContext> bmContext;
  std::shared_ptr<BMNNNetwork> bmNetwork;
  bm_handle_t handle;

  std::vector<float> mean;  // 前处理均值， 长度为3，顺序为rgb
  std::vector<float> stdd;  // 前处理方差， 长度为3，顺序为rgb
  bool bgr2rgb;             // 是否将bgr图像转成rgb推理

  // tpu_kernel
  bool use_tpu_kernel = false;
  bool has_init = false;
  tpu_kernel_api_yolov5NMS_t api[MAX_BATCH];
  tpu_kernel_function_t func_id;
  bm_device_mem_t out_dev_mem[MAX_BATCH];
  bm_device_mem_t detect_num_mem[MAX_BATCH];
  float* output_tensor[MAX_BATCH];
  int32_t detect_num[MAX_BATCH];

  float thresh_conf_min = 1;
  std::unordered_map<std::string, float> thresh_conf;  // 置信度阈值
  float thresh_nms;                                    // nms iou阈值
  std::vector<std::string> class_names;
  bool class_thresh_valid = false;
  float
      log_conf_threshold;  // 应用log运算符到阈值可在box过滤时省略box置信度的sigmoid计算

  int class_num = 80;  // default is coco names
  int m_frame_h, m_frame_w;
  int net_h, net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  int min_dim;
  bmcv_convert_to_attr converto_attr;

  bmcv_rect_t roi;
  bool roi_predefined = false;
  int thread_number;
  unsigned int m_max_det = UINT_MAX, m_min_det = 0;
};
}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV5_CONTEXT_H_