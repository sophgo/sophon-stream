#pragma once

#include "../Context.h"
#include "../share/common/bmnn_utils.h"
#include "../share/common/bm_wrapper.hpp"
#include "../share/common/ff_decode.hpp"
//#include "aiModelManagerWrapper.h" //neural network related APIs

#define USE_ASPECT_RATIO

namespace sophon_stream {
namespace algorithm {
namespace context {

float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *pIsAligWidth);


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

struct SophgoContext :public Context {
    std::vector<std::pair<int,std::vector<std::vector<float>>>> boxes; // 输出结果
    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    common::ErrorCode init(const std::string& json) override;

    ~SophgoContext();

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


    std::vector<float> m_thresh; // json --> Context --> SophgoContext

    int m_class_num = 80; // default is coco names
    int m_frame_h, m_frame_w;
    int m_net_h, m_net_w, m_net_channel;
    int max_batch;
    int input_num;
    int output_num;
    int min_dim;
    bmcv_convert_to_attr converto_attr;
    bool mEndOfStream = false;
    
};
} // namespace context
} // namespace algorithm
} // namespace sophon_stream



