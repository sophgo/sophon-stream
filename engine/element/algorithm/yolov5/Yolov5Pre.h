#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Yolov5SophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

class Yolov5Pre {
 public:
  /**
   * 执行预处理
   * @param[in] objectMetadatas:  输入数据
   * @param[out] context: 传输给推理模型的数据
   * @return 错误码
   */
  common::ErrorCode preProcess(Yolov5SophgoContext& context,
                               common::ObjectMetadatas& objectMetadatas);

  void initTensors(Yolov5SophgoContext& context,
                   common::ObjectMetadatas& objectMetadatas);

 private:
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream