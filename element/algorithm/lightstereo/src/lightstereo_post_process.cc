//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "lightstereo_post_process.h"

namespace sophon_stream {
namespace element {
namespace lightstereo {

void LightstereoPostProcess::init(std::shared_ptr<LightstereoContext> context) {}

void LightstereoPostProcess::postProcess(std::shared_ptr<LightstereoContext> context,
                                    common::ObjectMetadatas& objectMetadatas) {
  if (objectMetadatas.size() == 0) return;

  int ret = 0;
  for (auto obj : objectMetadatas){
    if (obj->mFrame->mEndOfStream) break;
    std::shared_ptr<BMNNTensor> outputTensor = std::make_shared<BMNNTensor>(
                                                obj->mOutputBMtensors->handle,
                                                context->bmNetwork->m_netinfo->output_names[0],
                                                context->bmNetwork->m_netinfo->output_scales[0],
                                                obj->mOutputBMtensors->tensors[0].get(),
                                                context->bmNetwork->is_soc);
    float* output_data = (float*)outputTensor->get_cpu_data();
    cv::Mat float_mat(context->net_h, context->net_w, CV_32FC1, output_data);
    cv::Mat normalized_disp_pred;
    if(obj->mFrame->mWidth <= context->net_w && obj->mFrame->mHeight <= context->net_h){
      cv::Rect bound = cv::Rect{0, context->net_h - obj->mFrame->mHeight, obj->mFrame->mWidth, obj->mFrame->mHeight};
      cv::Mat clipped = float_mat(bound);
      double minVal, maxVal;
      cv::minMaxLoc(clipped, &minVal, &maxVal);
      clipped.convertTo(normalized_disp_pred, CV_8UC1, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    }else{
      cv::Mat resized(obj->mFrame->mHeight, obj->mFrame->mWidth, CV_32FC1);
      cv::resize(float_mat, resized, cv::Size(obj->mFrame->mWidth, obj->mFrame->mHeight));
      double minVal, maxVal;
      cv::minMaxLoc(resized, &minVal, &maxVal);
      resized.convertTo(normalized_disp_pred, CV_8UC1, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    }
    cv::Mat color_normalized_disp_pred;
    cv::cvtColor(normalized_disp_pred, color_normalized_disp_pred, cv::COLOR_GRAY2BGR);
    std::shared_ptr<bm_image> output_bmimg = nullptr;
    output_bmimg.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    obj->mFrame->mSpData = output_bmimg;
    //For one chip:
    cv::bmcv::toBMI(color_normalized_disp_pred, output_bmimg.get(), true);
    obj->mFrame->mMat = color_normalized_disp_pred;
  
    //For pcie multi chips:
    // ret = bm_image_create(context->handle, obj->mFrame->mHeight, obj->mFrame->mWidth, FORMAT_BGR_PACKED, DATA_TYPE_EXT_1N_BYTE, output_bmimg.get());
    // STREAM_CHECK(ret == 0, "Alloc Device Memory Failed! Program Terminated.");
    // void* buffers[1] = {NULL};
    // buffers[0] = color_normalized_disp_pred.data;
    // ret = bm_image_copy_host_to_device(*output_bmimg, buffers);
  }
}

}  // namespace lightstereo
}  // namespace element
}  // namespace sophon_stream