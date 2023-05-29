//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../share/common/bm_wrapper.hpp"
#include "../share/common/bmnn_utils.h"
#include "../share/common/ff_decode.hpp"
#include "common/ErrorCode.h"

namespace sophon_stream {
namespace element {
namespace alg {

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);

  struct nodeDims {
    int c = 0;
    int h = 0;
    int w = 0;
  };

  struct AlgSophgoContext {
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

    ~AlgSophgoContext();

    std::shared_ptr<BMNNContext> m_bmContext;
    std::shared_ptr<BMNNNetwork> m_bmNetwork;
    std::vector<bm_image> m_resized_imgs;
    std::vector<bm_image> m_converto_imgs;
    bm_handle_t handle;

    bool use_tpu_kernel = false;

    std::vector<float> m_thresh;

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

}  // namespace alg
}  // namespace element
}  // namespace sophon_stream