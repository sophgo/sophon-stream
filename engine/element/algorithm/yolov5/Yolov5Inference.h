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

class Yolov5Inference {
 public:
  ~Yolov5Inference();
  /**
   * init device and engine
   * @param[in] context: model path,inputs and outputs name...
   */
  common::ErrorCode init(Yolov5SophgoContext& context);

  /**
   * network predict output
   * @param[in] context: inputData and outputData
   */
  common::ErrorCode predict(Yolov5SophgoContext& context,
                            common::ObjectMetadatas& objectMetadatas);

  void uninit();
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream