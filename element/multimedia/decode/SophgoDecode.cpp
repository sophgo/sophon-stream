#include "SophgoDecode.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

#include "SophgoContext.h"
#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/logger.h"
#include "common/type_trans.hpp"
#include "libyuv.h"
#include "opencv2/opencv.hpp"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace sophon_stream {
namespace element {
namespace decode {

SophgoDecode::SophgoDecode() {}

/**
 * 执行初始化
 * @param[in/out] context: 传输给推理模型的数据
 * @return 错误码
 */
common::ErrorCode SophgoDecode::init(SophgoContext& context) {
  mUrl = context.mUrl;
  mSourceType = context.mSourceType;
  mResizeRate = context.mResizeRate;
  int ret = bm_dev_request(&m_handle, context.deviceId);
  assert(BM_SUCCESS == ret);
  decoder.openDec(&m_handle, mUrl.c_str()); // 来一个任务 加一个decoder
  return common::ErrorCode::SUCCESS;
}

void bm_image2Frame(std::shared_ptr<common::Frame>& f, bm_image& img) {
  f->mWidth = img.width;
  f->mHeight = img.height;
  f->mDataType = sophon_stream::common::data_bmcv2stream(img.data_type);
  f->mFormatType = sophon_stream::common::format_bmcv2stream(img.image_format);
  f->mChannel = 3;
  f->mDataSize = img.width * img.height * f->mChannel * sizeof(uchar);

  // f->mData = std::make_shared<void>(buffers);
}
/**
 * preprocess
 * @param[in] context: input and output config
 * @param[in] objectMetadatas: inputData
 * @return preprocess error code or common::ErrorCode::SUCCESS
 */
common::ErrorCode SophgoDecode::process(
    SophgoContext& context,
    std::shared_ptr<common::ObjectMetadata>& objectMetadata) {
  SophgoContext* pSophgoContext = &context;
  if (pSophgoContext == nullptr) return (common::ErrorCode)(-1);

  objectMetadata = std::make_shared<common::ObjectMetadata>();
  // bm_image *img = decoder.grab();
  int eof = 0;
  double timestamp = 0.0;
  int frame_id = 0;
  std::shared_ptr<bm_image> spBmImage = decoder.grab(frame_id, eof, timestamp);

  // sleep(1);

  objectMetadata->mFrame = std::make_shared<common::Frame>();
  objectMetadata->mFrame->mTimestamp = timestamp * 1000000;
  objectMetadata->mFrame->mFrameId = frame_id;

  objectMetadata->mFrame->mSpData = spBmImage;

  objectMetadata->mFrame->mHandle = m_handle;
  if (1 == eof) {
    std::cout << " last frame! " << std::endl;
    objectMetadata->mFrame->mEndOfStream = true;
    return common::ErrorCode::STREAM_END;
  } else {
    bm_image2Frame(objectMetadata->mFrame, *spBmImage);
  }
  return common::ErrorCode::SUCCESS;
}

/**
 * 执行uninit释放资源
 */
void SophgoDecode::uninit() {
  // if(mSpDecoder!=nullptr){
  //     mSpDecoder->destroy();
  // }
  // if(mSpFormatInput!=nullptr){
  //     mSpFormatInput->destroy();
  // }
}

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream
