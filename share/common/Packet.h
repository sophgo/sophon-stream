#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "Rational.h"

namespace sophon_stream {
namespace common {

enum class CodecType {
    UNKNOWN, 
    H264, 
    H265, 
    HEVC = H265, 
    JPEG, 
};

struct Packet {
    Packet()
        : mChannelId(-1), 
          mPacketId(01), 
          mCodecType(CodecType::UNKNOWN), 
          mTimestamp(0), 
          mKeyFrame(false), 
          mEndOfStream(false), 
          mWidth(0), 
          mHeight(0), 
          mDataSize(0) {
    }

    bool empty() const {
        return 0 == mDataSize
            || !mData;
    }
    
    int mChannelId;
    std::int64_t mPacketId;

    CodecType mCodecType;
    common::Rational mFrameRate;
    std::int64_t mTimestamp;
    bool mKeyFrame;
    bool mEndOfStream;

    std::string mSide;

    int mWidth;
    int mHeight;

    std::size_t mDataSize;
    std::shared_ptr<void> mData;
};

} // namespace common
} // namespace sophon_stream

