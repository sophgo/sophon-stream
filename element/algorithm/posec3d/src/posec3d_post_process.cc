//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "posec3d_post_process.h"

#include <cmath>
#include <iostream>
#include <string>

#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace posec3d {

void Posec3dPostProcess::init(std::shared_ptr<Posec3dContext> context) {}

void Posec3dPostProcess::postProcess(std::shared_ptr<Posec3dContext> context,
                                     common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;

  // get 1 batch data
  auto& obj = objectMetadatas[0];
  // stream end control
  if (obj->mFrame->mEndOfStream) return;

  // init output tensors
  std::vector<std::shared_ptr<BMNNTensor>> outputTensors(context->output_num);
  for (int i = 0; i < context->output_num; i++) {
    outputTensors[i] = std::make_shared<BMNNTensor>(
        obj->mOutputBMtensors->handle,
        context->bmNetwork->m_netinfo->output_names[i],
        context->bmNetwork->m_netinfo->output_scales[i],
        obj->mOutputBMtensors->tensors[i].get(), context->bmNetwork->is_soc);
  }

  auto out_tensor = outputTensors[0];
  float* output_data = (float*)out_tensor->get_cpu_data();
  int cls_num = out_tensor->get_shape()->dims[1];
  int class_id = 0;
  for (int j = 0; j < cls_num; j++) {
    if (*(output_data + j) > *(output_data + class_id)) class_id = j;
  }
  float confidence = *(output_data + class_id);
  std::string res = context->class_names[class_id];
  std::shared_ptr<common::RecognizedObjectMetadata> recData =
      std::make_shared<common::RecognizedObjectMetadata>();
  recData->mLabelName = res;
  recData->mScores.push_back(confidence);
  obj->mRecognizedObjectMetadatas.push_back(recData);
}

}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream