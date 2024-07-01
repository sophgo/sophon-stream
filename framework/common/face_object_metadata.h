//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_FACE_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_FACE_OBJECT_METADATA_H_

#include <memory>
#include <string>
#include <vector>

namespace sophon_stream {
namespace common {
struct FaceObjectMetadata {
    int top;
    int bottom;
    int left;
    int right;
    float points_x[5];
    float points_y[5];
    float score;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_POSED_OBJECT_METADATA_H_