//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_RETINAFACE_POST_PROCESS_H_
#define SOPHON_STREAM_ELEMENT_RETINAFACE_POST_PROCESS_H_

#include <opencv2/opencv.hpp>

#include "algorithmApi/post_process.h"
#include "retinaface_context.h"

using namespace std;
using namespace cv;

namespace sophon_stream {
namespace element {
namespace retinaface {

typedef struct _tag__stFaceRect {
  int top;
  int bottom;
  int left;
  int right;
  float points_x[5];
  float points_y[5];
  float score;
} stFaceRect;

struct anchor_win {
  float x_ctr;
  float y_ctr;
  float w;
  float h;
};

struct anchor_box {
  float x1;
  float y1;
  float x2;
  float y2;
};

struct FacePts {
  float x[5];
  float y[5];
};

struct FaceDetectInfo {
  float score;
  anchor_box rect;
  FacePts pts;
};

struct anchor_cfg {
 public:
  int STRIDE;
  std::vector<int> SCALES;
  int BASE_SIZE;
  std::vector<float> RATIOS;
  int ALLOWED_BORDER;

  anchor_cfg() {
    STRIDE = 0;
    SCALES.clear();
    BASE_SIZE = 0;
    RATIOS.clear();
    ALLOWED_BORDER = 0;
  }
};

class RetinafacePostProcess : public ::sophon_stream::element::PostProcess {
 public:
  void init(std::shared_ptr<RetinafaceContext> context);
  /**
   * @brief 对一个batch的数据做后处理
   * @param context context指针
   * @param objectMetadatas 一个batch的数据
   */
  void postProcess(std::shared_ptr<RetinafaceContext> context,
                   common::ObjectMetadatas& objectMetadatas);

 private:
  anchor_box bbox_pred(anchor_box anchor, std::vector<float> regress);
  std::vector<anchor_box> bbox_pred(std::vector<anchor_box> anchors,
                                    std::vector<std::vector<float> > regress);
  std::vector<FacePts> landmark_pred(std::vector<anchor_box> anchors,
                                     std::vector<FacePts> facePts);
  FacePts landmark_pred(anchor_box anchor, FacePts facePt);
  static bool CompareBBox(const FaceDetectInfo& a, const FaceDetectInfo& b);
  std::vector<FaceDetectInfo> nms(std::vector<FaceDetectInfo>& bboxes,
                                  float threshold);
  void get_faceInfo(std::shared_ptr<RetinafaceContext> context,
                    vector<FaceDetectInfo>& faceInfo, float** preds,
                    map<string, int>& output_names_map, int img_h, int img_w,
                    float ratio_, float threshold, float scales = 1.0);

  // std::string network;
  std::vector<int> _feat_stride_fpn;
  std::map<std::string, std::vector<anchor_box> > _anchors_fpn;
  std::map<std::string, std::vector<anchor_box> > _anchors;
  std::map<std::string, int> _num_anchors;
};

}  // namespace retinaface
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_RETINAFACE_POST_PROCESS_H_