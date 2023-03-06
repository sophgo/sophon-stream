#pragma once

#include "../Context.h"
#include "../../share/common/bmnn_utils.h"
#include "../../share/common/bm_wrapper.hpp"
#include "../../share/common/ff_decode.hpp"
//#include "aiModelManagerWrapper.h" //neural network related APIs

namespace sophon_stream {
namespace algorithm {
namespace context {


struct SophgoContext :public Context {
    std::vector<std::pair<int,std::vector<std::vector<float>>>> boxes; // 输出结果
    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    common::ErrorCode init(const std::string& json) override;

    std::shared_ptr<BMNNContext> m_bmContext;
    std::shared_ptr<BMNNNetwork> m_bmNetwork;
    std::vector<bm_image> m_resized_imgs;
    std::vector<bm_image> m_converto_imgs;

    //configuration
    float m_confThreshold= 0.5;
    float m_nmsThreshold = 0.5;

    std::vector<std::string> m_class_names;
    int m_class_num = 80; // default is coco names
    int m_net_h, m_net_w;
    int max_batch;
    int output_num;
    int min_dim;
    bmcv_convert_to_attr converto_attr;
    
};
} // namespace context
} // namespace algorithm
} // namespace sophon_stream



