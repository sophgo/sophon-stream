#pragma once

#include "../PreProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

class Yolov5Pre : public algorithm::PreProcess {
  public :
    /**
     * 执行预处理
     * @param[in] objectMetadatas:  输入数据
     * @param[out] context: 传输给推理模型的数据
     * @return 错误码
     */
    common::ErrorCode preProcess(algorithm::Context& context,
                                 common::ObjectMetadatas& objectMetadatas) override;

  private:

};

} // namespace pre_process
} // namespace algorithm
} // namespace sophon_stream