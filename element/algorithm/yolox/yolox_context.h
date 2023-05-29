//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_
#define SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_

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
namespace yolox {

struct nodeDims {
  int c = 0;
  int h = 0;
  int w = 0;
};

struct YoloxContext {
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

  ~YoloxContext();

  std::shared_ptr<BMNNContext> m_bmContext;
  std::shared_ptr<BMNNNetwork> m_bmNetwork;
  std::vector<bm_image> m_resized_imgs;
  std::vector<bm_image> m_converto_imgs;
  bm_handle_t handle;

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
}  // namespace yolox
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOX_CONTEXT_H_