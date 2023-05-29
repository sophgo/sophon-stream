//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "TemplateSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace template {

  class TemplatePre {
   public:
    /**
     * 执行预处理
     * @param[in] objectMetadatas:  输入数据
     * @param[out] context: 传输给推理模型的数据
     * @return 错误码
     */
    common::ErrorCode preProcess(TemplateSophgoContext& context,
                                 common::ObjectMetadatas& objectMetadatas);

    void initTensors(TemplateSophgoContext& context,
                     common::ObjectMetadatas& objectMetadatas);
  };

}  // namespace template
}  // namespace element
}  // namespace sophon_stream