//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV7_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOV7_POST_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "common/error_code.h"
#include "common/object_metadata.h"
#include "group.h"
#include "yolov7_context.h"

namespace sophon_stream {
namespace element {
namespace yolov7 {

struct YoloV7Box {
  int x, y, width, height;
  float score;
  int class_id;
};

using YoloV7BoxVec = std::vector<YoloV7Box>;

typedef struct tpu_kernel_ {
  bool use_tpu_kernel = false;
  bool has_init = false;
  tpu_kernel_api_yolov7NMS_t api[MAX_BATCH];
  tpu_kernel_function_t func_id;
  bm_device_mem_t out_dev_mem[MAX_BATCH];
  bm_device_mem_t detect_num_mem[MAX_BATCH];
  float* output_tensor[MAX_BATCH];
  int32_t detect_num[MAX_BATCH];
} tpu_kernel;

class Yolov7PostProcess : public ::sophon_stream::framework::PostProcess {
 public:
  void init(std::shared_ptr<Yolov7Context> context);

  void postProcess(std::shared_ptr<Yolov7Context> context,
                   common::ObjectMetadatas& objectMetadatas, int dataPipeId);

  ~Yolov7PostProcess() override;

 private:
  tpu_kernel* multi_thread_tpu_kernel = nullptr;
  std::shared_ptr<Yolov7Context> global_context = nullptr;

  void setTpuKernelMem(std::shared_ptr<Yolov7Context> context,
                       common::ObjectMetadatas& objectMetadatas,
                       tpu_kernel& tpu_k);
  float sigmoid(float x);
  int argmax(float* data, int num);
  void NMS(YoloV7BoxVec& dets, float nmsConfidence);
  float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h,
                                bool* pIsAligWidth);
  void postProcessCPU(std::shared_ptr<Yolov7Context> context,
                      common::ObjectMetadatas& objectMetadatas);
  void postProcessTPUKERNEL(std::shared_ptr<Yolov7Context> context,
                            common::ObjectMetadatas& objectMetadatas,
                            int dataPipeId);
};

}  // namespace yolov7
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV7_POST_PROCESS_H_