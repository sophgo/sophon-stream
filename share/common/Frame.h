#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include "bmlib_runtime.h"
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "Rational.h"

namespace sophon_stream {
namespace common {

enum class FormatType {
    UNKNOWN, 
    NV12, 
    NV21, 
    BGR_PACKET, 
    BGR_PLANAR, 
    RGB_PACKET, 
    RGB_PLANAR, 
};

enum class DataType {
    INTEGER, 
    FLOATING_POINT, 
};

struct SophonData{
    SophonData(){
            mData.reset(new bm_device_mem_t, [&](bm_device_mem_t* p){
                bm_free_device(mHandle, *p);
                delete p;
        });
    }
    bm_handle_t mHandle;
    std::shared_ptr<bm_device_mem_t> mData;
};

struct Frame {
    Frame()
        : mChannelId(-1), 
          mFrameId(-1), 
          mFormatType(FormatType::UNKNOWN), 
          mDataType(DataType::INTEGER), 
          mTimestamp(0), 
          mEndOfStream(false), 
          mChannel(0), 
          mChannelStep(0), 
          mWidth(0), 
          mWidthStep(0), 
          mHeight(0), 
          mHeightStep(0), 
          mDataSize(0) {
    }

    bool empty() const {
        return 0 == mChannel
            || 0 == mChannelStep
            || 0 == mWidth
            || 0 == mWidthStep
            || 0 == mHeight
            || 0 == mHeightStep
            || 0 == mDataSize
            || !mSpData;
    }

    int mChannelId;
    int mChannelIdInternal;
    std::int64_t mFrameId;

    FormatType mFormatType;
    DataType mDataType;
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
    //std::shared_ptr<SophonData> mSpData;
    bm_handle_t mHandle;
    std::shared_ptr<bm_image> mSpData;
    std::shared_ptr<bm_image> mSpDataOsd;
};

} // namespace common
} // namespace sophon_stream

