//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_OBB_OBJECT_METADATA_H_
#define SOPHON_STREAM_COMMON_OBB_OBJECT_METADATA_H_

#include <memory>
#include <string>
#include <vector>


namespace sophon_stream {
namespace common {
struct ObbObjectMetadata {
    float x1, y1, x2, y2, x3, y3, x4, y4, score;
    int class_id;
    inline void add_offset(int x, int y){
        this->x1 += x;
        this->x2 += x;
        this->x3 += x;
        this->x4 += x;
        this->y1 += y;
        this->y2 += y;
        this->y3 += y;
        this->y4 += y;
    }
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_POSED_OBJECT_METADATA_H_