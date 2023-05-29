//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "AlgInference.h"

#include <fstream>

namespace sophon_stream {
namespace element {
namespace alg {

  AlgInference::~AlgInference() {}

  /**
   * init device and engine
   * @param[in] @param[in] context: model path,inputs and outputs name...
   */
  common::ErrorCode AlgInference::init(AlgSophgoContext & context) {
    
  }

  common::ErrorCode AlgInference::predict(
      AlgSophgoContext & context,
      common::ObjectMetadatas & objectMetadatas) {
    AlgSophgoContext* pSophgoContext = &context;

    if (objectMetadatas.size() == 0) return common::ErrorCode::SUCCESS;

    int ret = 0;
    if (!objectMetadatas[0]->mFrame->mEndOfStream)
      ret = pSophgoContext->m_bmNetwork->forward(
          objectMetadatas[0]->mInputBMtensors->tensors,
          objectMetadatas[0]->mOutputBMtensors->tensors);
    return static_cast<common::ErrorCode>(ret);
  }

}  // namespace alg
}  // namespace element
}  // namespace sophon_stream