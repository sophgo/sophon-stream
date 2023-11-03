//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_FRAME_H_
#define SOPHON_STREAM_COMMON_FRAME_H_

#include <cstddef>
#include <cstdint>
#include <memory>

// for bmcv_api_ext.h
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"

namespace sophon_stream {
namespace common {

struct Rational {
  Rational() : mNumber(0), mDenominator(1) {}

  Rational(int number, int denominator)
      : mNumber(number), mDenominator(denominator) {}

  int mNumber;
  int mDenominator;
};

struct Frame {
  Frame()
      : mChannelId(-1),
        mFrameId(-1),
        mFormatType(FORMAT_YUV420P),
        mDataType(DATA_TYPE_EXT_1N_BYTE),
        mTimestamp(0),
        mEndOfStream(false),
        mChannel(0),
        mChannelStep(0),
        mWidth(0),
        mWidthStep(0),
        mHeight(0),
        mHeightStep(0),
        mDataSize(0) {}

  bool empty() const {
    return 0 == mChannel || 0 == mChannelStep || 0 == mWidth ||
           0 == mWidthStep || 0 == mHeight || 0 == mHeightStep ||
           0 == mDataSize || !mSpData;
  }

  int mChannelId;
  int mChannelIdInternal;
  std::int64_t mFrameId;

  bm_image_format_ext mFormatType;
  bm_image_data_format_ext mDataType;
  Rational mFrameRate;
  std::int64_t mTimestamp;
  bool mEndOfStream;

  std::string mSide;

  int mChannel;
  int mChannelStep;
  int mWidth;
  int mWidthStep;
  int mHeight;
  int mHeightStep;
  std::size_t mDataSize;
  bm_handle_t mHandle;
  std::shared_ptr<bm_image> mSpData;
  std::shared_ptr<bm_image> mSpDataOsd;
};

}  // namespace common
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_COMMON_FRAME_H_