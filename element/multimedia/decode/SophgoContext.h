#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/bm_wrapper.hpp"
#include "common/bmnn_utils.h"
#include "common/ff_decode.hpp"
#include "common/ErrorCode.h"
#include "common/Graphics.hpp"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace decode {

struct SophgoContext {
  std::string multimediaName;
  int deviceId;
  std::shared_ptr<void> data = nullptr;

  std::string mUrl;
  common::Rectangle<int> mRoi;
  float mResizeRate = 1.0f;
  int mTimeout = 0;
  int mSourceType = 0;  // 0视频文件1是文件夹2是rtsp或rtmp

  static constexpr const char* JSON_URL = "url";
  static constexpr const char* JSON_RESIZE_RATE = "resize_rate";
  static constexpr const char* JSON_TIMEOUT = "timeout";
  static constexpr const char* JSON_CHANNELID = "channel_id";
  static constexpr const char* JSON_SOURCE_TYPE = "source_type";
  static constexpr const char* JSON_ROI = "roi";
  static constexpr const char* JSON_X = "x";
  static constexpr const char* JSON_Y = "y";
  static constexpr const char* JSON_W = "w";
  static constexpr const char* JSON_H = "h";

  std::vector<std::pair<int, std::vector<std::vector<float>>>>
      boxes;  // 输出结果
  /**
   * context初始化
   * @param[in] json: 初始化的json字符串
   * @return 错误码
   */
  common::ErrorCode init(const std::string& json);

  std::shared_ptr<BMNNContext> m_bmContext;
  std::shared_ptr<BMNNNetwork> m_bmNetwork;
  std::vector<bm_image> m_resized_imgs;
  std::vector<bm_image> m_converto_imgs;

  // configuration
  float m_confThreshold = 0.5;
  float m_nmsThreshold = 0.5;

  std::vector<std::string> m_class_names;
  int m_class_num = 80;  // default is coco names
  int m_net_h, m_net_w;
  int max_batch;
  int output_num;
  int min_dim;
  bmcv_convert_to_attr converto_attr;
};
}  // namespace decode
}  // namespace element
}  // namespace sophon_stream
