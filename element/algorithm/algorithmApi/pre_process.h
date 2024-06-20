//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_ALGORITHMAPI_PREPROCESS_H_
#define SOPHON_STREAM_ELEMENT_ALGORITHMAPI_PREPROCESS_H_

#include "context.h"

namespace sophon_stream {
namespace element {

class PreProcess {
 public:
  PreProcess() = default;
  virtual ~PreProcess() = default;

  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth) {
    float ratio;
    float r_w = (float)dst_w / src_w;
    float r_h = (float)dst_h / src_h;
    if (r_h > r_w) {
      *pIsAligWidth = true;
      ratio = r_w;
    } else {
      *pIsAligWidth = false;
      ratio = r_h;
    }
    return ratio;
  }

  template <typename T, typename U = Context,
            typename std::enable_if<std::is_base_of<U, T>::value, int>::type* =
                nullptr>
  void initTensors(std::shared_ptr<T> context,
                   common::ObjectMetadatas& objectMetadatas) {
    for (auto& obj : objectMetadatas) {
      obj->mInputBMtensors =
          std::make_shared<sophon_stream::common::bmTensors>();
      int channelId = obj->mFrame->mChannelId;
      int frameId = obj->mFrame->mFrameId;
      obj->mInputBMtensors.reset(
          new sophon_stream::common::bmTensors(),
          [channelId, frameId](sophon_stream::common::bmTensors* p) {
            for (int i = 0; i < p->tensors.size(); ++i) {
              if (p->tensors[i]->device_mem.u.device.device_addr != 0) {
                bm_free_device(p->handle, p->tensors[i]->device_mem);
              }
            }

            delete p;
            p = nullptr;
          });
      obj->mInputBMtensors->handle = context->handle;
      obj->mInputBMtensors->tensors.resize(context->input_num);
      for (int i = 0; i < context->input_num; ++i) {
        obj->mInputBMtensors->tensors[i] = std::make_shared<bm_tensor_t>();
        obj->mInputBMtensors->tensors[i]->dtype =
            context->bmNetwork->m_netinfo->input_dtypes[i];
        obj->mInputBMtensors->tensors[i]->shape =
            context->bmNetwork->m_netinfo->stages[0].input_shapes[i];
        obj->mInputBMtensors->tensors[i]->shape.dims[0] = 1;
        obj->mInputBMtensors->tensors[i]->st_mode = BM_STORE_1N;
      }
    }
  }
};
}  // namespace element
}  // namespace sophon_stream

#endif