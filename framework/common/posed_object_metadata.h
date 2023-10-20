//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_POSED_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_POSED_OBJECT_METADATA_H_

#include <memory>
#include <string>
#include <vector>

#include "graphics.h"

namespace sophon_stream {
namespace common {
struct PosedObjectMetadata {
  enum EModelType {
    BODY_25 = 0,
    COCO_18 = 1
  };  // 检测模型类别，支持25个关键点或者18个
  std::vector<float>
      keypoints;  //{x,y,score....},包含一个人的所有关键点坐标，大小与modeltype有关
  std::vector<float> scores;
  EModelType modeltype;  // 模型类别
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_POSED_OBJECT_METADATA_H_