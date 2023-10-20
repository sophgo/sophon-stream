//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_FASTPOSE_INFERENCE_H_
#define SOPHON_STREAM_ELEMENT_FASTPOSE_INFERENCE_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "fastpose_context.h"

namespace sophon_stream {
namespace element {
namespace fastpose {

class FastposeInference {
 public:
  ~FastposeInference();
  /**
   * @brief init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  void init(std::shared_ptr<FastposeContext> context);

  /**
   * @brief network predict output
   * @param[in] context: inputData and outputData
   * @param[in] objectMetadatas: 一个batch的数据
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode predict(std::shared_ptr<FastposeContext> context,
                            common::ObjectMetadatas& objectMetadatas);

 private:
  /**
   * @brief 组合inputTensor，batchsize==1时不调用
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   * @return std::shared_ptr<sophon_stream::common::bmSubTensors>
   * 组合的inputTensors，其中的输入对应每个box
   */
  std::shared_ptr<sophon_stream::common::bmSubTensors> mergeInputDeviceMem(
      std::shared_ptr<FastposeContext> context,
      common::ObjectMetadatas& objectMetadatas);
  /**
   * @brief 申请outputTensors
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   * @return std::shared_ptr<sophon_stream::common::bmSubTensors>
   * 申请的outputTensors
   */
  std::shared_ptr<sophon_stream::common::bmSubTensors> getOutputDeviceMem(
      std::shared_ptr<FastposeContext> context,
      common::ObjectMetadatas& objectMetadatas);
  /**
   * @brief
   * 将更新的outputTensors分配到每一个ObjectMetadata上，batchsize==1时不调用
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   * @param outputTensors 经过推理，更新的outputTensors
   */
  void splitOutputMemIntoObjectMetadatas(
      std::shared_ptr<FastposeContext> context,
      common::ObjectMetadatas& objectMetadatas,
      std::shared_ptr<sophon_stream::common::bmSubTensors>& outputTensors);
};

}  // namespace fastpose
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_FASTPOSE_INFERENCE_H_