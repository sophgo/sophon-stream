#pragma once

#include <memory>

#include "../../../framework/ElementNew.h"
#include "UnetInference.h"
#include "UnetPost.h"
#include "UnetPre.h"
#include "UnetSophgoContext.h"

namespace sophon_stream {
namespace algorithm {
class UnetAlgorithm : public ::sophon_stream::framework::Element {
 public:
  UnetAlgorithm();

  ~UnetAlgorithm() override;

  UnetAlgorithm(const UnetAlgorithm&) = delete;
  UnetAlgorithm& operator=(const UnetAlgorithm&) = delete;
  UnetAlgorithm(UnetAlgorithm&&) = default;
  UnetAlgorithm& operator=(UnetAlgorithm&&) = default;

  /**
   * 初始化函数
   * @param[in] side:  设备类型
   * @param[in] deviceId:  设备ID
   * @param[in] json:  初始化字符串
   * @return 错误码
   */
  common::ErrorCode initInternal(const std::string& json) override;
  /**
   * 预测函数
   * @param[in/out] objectMetadatas:  输入数据和预测结果
   */
  void process(common::ObjectMetadatas& objectMetadatas);
  /**
   * 资源释放函数
   */
  void uninitInternal() override;

  common::ErrorCode doWork() override;
  /**
   * @brief 从processed objectmetadatas队列中获取数据并发送至0通道
   */
  common::ErrorCode sendProcessedData();

  // static constexpr const char* NAME = "yolov5_algorithm";

 private:
  std::shared_ptr<unet::UnetSophgoContext> mContext;  // context对象
  std::shared_ptr<unet::UnetPre> mPreProcess;         // 预处理对象
  std::shared_ptr<unet::UnetInference> mInference;    // 推理对象
  std::shared_ptr<unet::UnetPost> mPostProcess;       // 后处理对象

  bool use_pre = false;
  bool use_infer = false;
  bool use_post = false;

  bool hasEof = false;

  /**
   * @brief 需要推理的batch数
   */
  int mBatch;

  /**
   * @brief 上一次推理前datapipe中dataqueue的size
   */
  int mLastDataCount;

  /**
   * @brief 发送数据缓冲
   */
  std::deque<std::shared_ptr<common::ObjectMetadata>> mProcessedObjectMetadatas;

  /**
   * @brief batch数据缓冲
   */
  common::ObjectMetadatas mPendingObjectMetadatas;

  std::mutex mMutex2;
};

}  // namespace algorithm
}  // namespace sophon_stream
