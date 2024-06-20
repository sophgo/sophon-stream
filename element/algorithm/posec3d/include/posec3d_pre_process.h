//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_POSEC3D_PRE_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_POSEC3D_PRE_PROCESS_H_

#include "algorithmApi/pre_process.h"
#include "posec3d_context.h"

namespace sophon_stream {
namespace element {
namespace posec3d {

using fpptr_dim3 = std::vector<
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>>>;
using fpptr_dim2 = std::vector<std::shared_ptr<std::vector<float>>>;

class Posec3dPreProcess : public ::sophon_stream::element::PreProcess {
 public:
  /**
   * @brief 对一个batch的数据做预处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode preProcess(std::shared_ptr<Posec3dContext> context,
                               common::ObjectMetadatas& objectMetadatas);
  void init(std::shared_ptr<Posec3dContext> context);

 private:
  /**
   * @brief 对一个batch的数据做重采样
   * @param sampled_keypoints 重采样关键点结果
   * @param sampled_keypoint_scores 重采样关键点置信度结果
   * @param clip_len 每次裁剪的长度
   * @param num_clips 裁剪次数
   * @param seed 当帧数大于裁剪长度时裁剪时的随机种子
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode uniformSampleFrames(fpptr_dim3& sampled_keypoints,
                                        fpptr_dim3& sampled_keypoint_scores,
                                        int clip_len, int num_clips, int seed);

  /**
   * @brief 对一个batch的关键点数据做偏移并生成下面预处理所需要的参数
   * @param objectMetadatas 一个batch的数据
   * @param keypoints 关键点输入
   * @param padding padding尺寸
   * @param threshold 长宽阈值
   * @param hw_ratio 长宽比率
   * @param new_shape 输出的长宽
   * @param crop_quadruple 输出的被裁剪四边形几何属性
   * @param allow_imgpad 是否进行输出长宽的裁剪
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode poseCompact(common::ObjectMetadatas& objectMetadatas,
                                fpptr_dim3& keypoints, float padding,
                                int threshold, std::vector<float>& hw_ratio,
                                std::vector<int>& new_shape,
                                std::vector<float>& crop_quadruple,
                                bool allow_imgpad);

  /**
   * @brief 对一个batch的关键点数据做缩放，并生成下面预处理所需要的参数
   * @param keypoints 关键点输入
   * @param scale 缩放尺寸
   * @param new_shape poseCompact输出的长宽，最终为输出的新长宽
   * @param keep_ratio 是否缩放时保持纵横比
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode resize(fpptr_dim3& keypoints, std::vector<int>& scale,
                           std::vector<int>& new_shape, bool keep_ratio);

  /**
   * @brief 对一个batch的关键点数据做偏移，并生成下面预处理所需要的参数
   * @param keypoints 关键点输入
   * @param new_shape resize输出的新长宽
   * @param crop_quadruple 输出的新几何属性
   * @param crop_size 裁剪尺寸
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode centerCrop(fpptr_dim3& keypoints,
                               std::vector<int>& new_shape,
                               std::vector<float>& crop_quadruple,
                               std::vector<int>& crop_size);

  /**
   * @brief 生成模型输入heatmap
   * @param context context指针
   * @param sampled_keypoints 来自uniformSampleFrames输出的重采样关键点结果
   * @param sampled_keypoint_scores 关键点输入
   * @param new_shape centerCrop输出的新长宽
   * @param heatmap 输出heatmap的指针
   * @param out_num 输出heatmap的长度
   * @param sigma 缩放因子
   * @param scaling 缩放因子
   * @param clip_len 每次裁剪的长度
   * @return common::ErrorCode
   * common::ErrorCode::SUCCESS，中间过程失败会中断执行
   */
  common::ErrorCode generatePoseTarget(std::shared_ptr<Posec3dContext> context,
                                       fpptr_dim3& keypoints,
                                       fpptr_dim3& sampled_keypoints,
                                       fpptr_dim3& sampled_keypoint_scores,
                                       std::vector<int>& new_shape,
                                       float* heatmap, int out_num, float sigma,
                                       float scaling, int clip_len);

  /**
   * @brief 为一个batch的数据初始化设备内存
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void initTensors(std::shared_ptr<Posec3dContext> context,
                   common::ObjectMetadatas& objectMetadatas);
};

}  // namespace posec3d
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_POSEC3D_PRE_PROCESS_H_