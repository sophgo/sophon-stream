//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//


#ifndef SOPHON_STREAM_COMMON_TOOL_H_
#define SOPHON_STREAM_COMMON_TOOL_H_

#include "opencv2/opencv.hpp"
#include "common/common_defs.h"

int save_frame_to_yuv(AVFrame *frame, const char *filename);

#endif  // SOPHON_STREAM_COMMON_TOOL_H_
