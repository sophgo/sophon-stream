//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_YOLOV5_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_YOLOV5_POST_PROCESS_H_

#include "algorithmApi/post_process.h"
#include "yolov5_context.h"

namespace sophon_stream {
namespace element {
namespace yolov5 {

struct YoloV5Box {
  int x, y, width, height;
  float score;
  int class_id;
};

using YoloV5BoxVec = std::vector<YoloV5Box>;

typedef struct tpu_kernel_ {
  bool use_tpu_kernel = false;
  bool has_init = false;
  tpu_kernel_api_yolov5NMS_t api[MAX_BATCH];
  tpu_kernel_function_t func_id;
  bm_device_mem_t out_dev_mem[MAX_BATCH];
  bm_device_mem_t detect_num_mem[MAX_BATCH];
  float* output_tensor[MAX_BATCH];
  int32_t detect_num[MAX_BATCH];
} tpu_kernel;

class Yolov5PostProcess : public ::sophon_stream::element::PostProcess {
 public:
  void init(std::shared_ptr<Yolov5Context> context);

  void postProcess(std::shared_ptr<Yolov5Context> context,
                   common::ObjectMetadatas& objectMetadatas, int dataPipeId);

  ~Yolov5PostProcess() override;

 private:
  tpu_kernel* multi_thread_tpu_kernel = nullptr;
  std::shared_ptr<Yolov5Context> global_context = nullptr;

  void setTpuKernelMem(std::shared_ptr<Yolov5Context> context,
                       common::ObjectMetadatas& objectMetadatas,
                       tpu_kernel& tpu_k);
  float sigmoid(float x);
  int argmax(float* data, int num);
  void NMS(YoloV5BoxVec& dets, float nmsConfidence);
  void postProcessCPU(std::shared_ptr<Yolov5Context> context,
                      common::ObjectMetadatas& objectMetadatas);
  void postProcessTPUKERNEL(std::shared_ptr<Yolov5Context> context,
                            common::ObjectMetadatas& objectMetadatas,
                            int dataPipeId);
};

}  // namespace yolov5
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_YOLOV5_POST_PROCESS_H_