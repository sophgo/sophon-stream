#include "decoder.h"

#include <nlohmann/json.hpp>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {
namespace decode {


void bm_image2Frame(std::shared_ptr<common::Frame>& f, bm_image& img) {
  f->mWidth = img.width;
  f->mHeight = img.height;
  f->mDataType = img.data_type;
  f->mFormatType = img.image_format;
  f->mChannel = 3;
  f->mDataSize = img.width * img.height * f->mChannel * sizeof(uchar);
}

Decoder::Decoder() {}

Decoder::~Decoder() {}

common::ErrorCode Decoder::init(int deviceId, const std::string& url) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    mUrl = url;

    int ret = bm_dev_request(&m_handle, deviceId);
    mDeviceId = deviceId;
    assert(BM_SUCCESS == ret);
    decoder.openDec(&m_handle, mUrl.c_str());

  } while (false);

  return errorCode;
}

common::ErrorCode Decoder::process(
    std::shared_ptr<common::ObjectMetadata>& objectMetadata) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  int frame_id = 0;
  int eof = 0;
  std::shared_ptr<bm_image> spBmImage = decoder.grab(frame_id, eof);

  objectMetadata = std::make_shared<common::ObjectMetadata>();
  objectMetadata->mFrame = std::make_shared<common::Frame>();

  objectMetadata->mFrame->mHandle = m_handle;
  objectMetadata->mFrame->mFrameId = frame_id;
  objectMetadata->mFrame->mSpData = spBmImage;
  if (eof) {
    objectMetadata->mFrame->mEndOfStream = true;
    errorCode = common::ErrorCode::STREAM_END;
  } else {
    bm_image2Frame(objectMetadata->mFrame, *spBmImage);
  }

  if (common::ErrorCode::SUCCESS != errorCode) {
    objectMetadata->mErrorCode = errorCode;
  }

  return errorCode;
}

void Decoder::uninit() {}

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream