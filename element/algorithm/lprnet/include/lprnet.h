//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_LPRNET_H_
#define SOPHON_STREAM_ELEMENT_LPRNET_H_

#include <fstream>
#include <memory>
#include <mutex>

#include "common/profiler.h"
#include "element.h"
#include "lprnet_context.h"
#include "lprnet_inference.h"
#include "lprnet_post_process.h"
#include "lprnet_pre_process.h"

namespace sophon_stream {
namespace element {
namespace lprnet {

  class Lprnet : public ::sophon_stream::framework::Element {
   public:
    Lprnet();
    ~Lprnet() override;

    /**
     * @brief
     * 解析configure，初始化派生element的特有属性；调用initContext初始化算法相关参数
     * @param json json格式的配置文件
     * @return common::ErrorCode
     * 成功返回common::ErrorCode::SUCCESS，失败返回common::ErrorCode::PARSE_CONFIGURE_FAIL
     */
    common::ErrorCode initInternal(const std::string& json) override;

    /**
     * @brief
     * element的功能在这里实现。例如，算法模块需要实现组batch、调用算法、发送数据等功能
     * @param dataPipeId pop数据时对应的dataPipeId
     * @return common::ErrorCode 成功返回common::ErrorCode::SUCCESS
     */
    common::ErrorCode doWork(int dataPipeId) override;

    /**
     * @brief 从json文件读取的配置项
     */
    static constexpr const char* CONFIG_INTERNAL_STAGE_NAME_FIELD = "stage";
    static constexpr const char* CONFIG_INTERNAL_MODEL_PATH_FIELD =
        "model_path";

   private:
    std::shared_ptr<LprnetContext> mContext;          // context对象
    std::shared_ptr<LprnetPreProcess> mPreProcess;    // 预处理对象
    std::shared_ptr<LprnetInference> mInference;      // 推理对象
    std::shared_ptr<LprnetPostProcess> mPostProcess;  // 后处理对象

    bool use_pre = false;
    bool use_infer = false;
    bool use_post = false;
    int mBatch;

    std::string mFpsProfilerName;
    ::sophon_stream::common::FpsProfiler mFpsProfiler;

    common::ErrorCode initContext(const std::string& json);
    void process(common::ObjectMetadatas& objectMetadatas);
  };

}  // namespace lprnet
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_LPRNET_H_