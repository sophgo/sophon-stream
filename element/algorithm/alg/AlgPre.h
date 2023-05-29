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

#include "AlgSophgoContext.h"
#include "common/ErrorCode.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace alg {

  class AlgPre {
   public:
    /**
     * 执行预处理
     * @param[in] objectMetadatas:  输入数据
     * @param[out] context: 传输给推理模型的数据
     * @return 错误码
     */
    common::ErrorCode preProcess(AlgSophgoContext& context,
                                 common::ObjectMetadatas& objectMetadatas);

    void initTensors(AlgSophgoContext& context,
                     common::ObjectMetadatas& objectMetadatas);
  };

}  // namespace alg
}  // namespace element
}  // namespace sophon_stream