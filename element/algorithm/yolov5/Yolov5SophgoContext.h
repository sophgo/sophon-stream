//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_SOPHGOCONTEXT_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_SOPHGOCONTEXT_H_

#include <memory>
#include <string>
#include <vector>

#include "common/ErrorCode.h"
#include "common/bm_wrapper.hpp"
#include "common/bmnn_utils.h"
#include "common/ff_decode.hpp"

#define USE_ASPECT_RATIO

namespace sophon_stream {
namespace element {
namespace yolov5 {

float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                              bool* pIsAligWidth);

#define MAX_YOLO_INPUT_NUM 3
#define MAX_YOLO_ANCHOR_NUM 3
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
} tpu_kernel_api_yolov5NMS_t;

#define MAX_BATCH 16

struct nodeDims {
  int c = 0;
  int h = 0;
  int w = 0;
};

struct Yolov5SophgoContext {
  std::string algorithmName;                // 算法名字
  int deviceId;                             // 设备ID
  int maxBatchSize;                         // 最大batch
  int numBatch;                             // 当前batch
  int numClass;                             // 类别数目
  std::vector<std::string> modelPath;       // 模型路径
  std::vector<std::string> inputNodeName;   // 输入节点名字
  std::vector<nodeDims> inputShape;         // 输入shape
  std::vector<std::string> outputNodeName;  // 输出节点名字
  std::vector<nodeDims> outputShape;        // 输出shape
  std::vector<int> numInputs;               // 输入数量
  std::vector<int> numOutputs;              // 输出数量
  std::vector<float> threthold;             // 阈值
  std::vector<std::string> labelNames;      // lablel名字
  std::shared_ptr<void> data = nullptr;     // 数据
                                            //
  std::vector<std::pair<int, std::vector<std::vector<float>>>>
      boxes;  // 输出结果
  /**
   * context初始化
   * @param[in] json: 初始化的json字符串
   * @return 错误码
   */
  common::ErrorCode init(const std::string& json);

  ~Yolov5SophgoContext();

  std::shared_ptr<BMNNContext> m_bmContext;
  std::shared_ptr<BMNNNetwork> m_bmNetwork;
  std::vector<bm_image> m_resized_imgs;
  std::vector<bm_image> m_converto_imgs;
  bm_handle_t handle;

  // tpu_kernel
  bool use_tpu_kernel = false;
  bool has_init = false;
  tpu_kernel_api_yolov5NMS_t api[MAX_BATCH];
  tpu_kernel_function_t func_id;
  bm_device_mem_t out_dev_mem[MAX_BATCH];
  bm_device_mem_t detect_num_mem[MAX_BATCH];
  float* output_tensor[MAX_BATCH];
  int32_t detect_num[MAX_BATCH];

  std::vector<float> m_thresh;  // json --> Context --> SophgoContext

  int m_class_num = 80;  // default is coco names
  int m_frame_h, m_frame_w;
  int m_net_h, m_net_w, m_net_channel;
  int max_batch;
  int input_num;
  int output_num;
  int min_dim;
  bmcv_convert_to_attr converto_attr;
  bool mEndOfStream = false;
};
}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_YOLOV5_SOPHGOCONTEXT_H_