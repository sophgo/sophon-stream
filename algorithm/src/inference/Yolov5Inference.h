#pragma once

#include "../Inference.h"

namespace sophon_stream {
namespace algorithm {
namespace inference {

/**
 * gpu版本yolov3预处理
 */
class Yolov5Inference : public algorithm::Inference {
  public:
    ~Yolov5Inference();
    /**
     * init device and engine
     * @param[in] context: model path,inputs and outputs name...
     */
    common::ErrorCode init(algorithm::Context& context) override;

    /**
     * network predict output
     * @param[in] context: inputData and outputData
     */
    common::ErrorCode predict(algorithm::Context& context) override;

    void uninit() override;

  private:
    // std::shared_ptr<BMNNContext> m_bmContext;
    // std::shared_ptr<BMNNNetwork> m_bmNetwork;
    // std::vector<bm_image> m_resized_imgs;
    // std::vector<bm_image> m_converto_imgs;

    // //configuration
    // float m_confThreshold= 0.5;
    // float m_nmsThreshold = 0.5;

    // std::vector<std::string> m_class_names;
    // int m_class_num = 80; // default is coco names
    // int m_net_h, m_net_w;
    // int max_batch;
    // int output_num;
    // int min_dim;
    // bmcv_convert_to_attr converto_attr;
};

} // namespace inference
} // namespace algorithm
} // namespace sophon_stream