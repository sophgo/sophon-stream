#pragma once

#include <memory>

#include "SophgoContext.h"
#include "SophgoDecode.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 多媒体模块
 */
class Decoder {
 public:
  /**
   * 构造函数
   */
  Decoder();
  /**
   * 析构函数
   */
  ~Decoder();

  Decoder(const Decoder&) = delete;
  Decoder& operator=(const Decoder&) = delete;
  Decoder(Decoder&&) = default;
  Decoder& operator=(Decoder&&) = default;

  /**
   * 初始化函数
   * @param[in] side:  设备类型
   * @param[in] deviceId:  设备ID
   * @param[in] json:  初始化字符串
   * @return 错误码
   */
  common::ErrorCode init(const std::string& side, int deviceId,
                         const std::string& json);
  /**
   * 预测函数
   * @param[in/out] objectMetadatas:  输入数据和预测结果
   */
  common::ErrorCode process(
      std::shared_ptr<common::ObjectMetadata>& objectMetadata);
  /**
   * 资源释放函数
   */
  void uninit();

 private:
  std::shared_ptr<decode::SophgoContext> mContext;
  std::shared_ptr<decode::SophgoDecode> mProcess;
};

}  // namespace multimedia
}  // namespace sophon_stream
