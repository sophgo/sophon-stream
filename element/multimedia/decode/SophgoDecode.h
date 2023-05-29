#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/ff_decode.hpp"
#include "SophgoContext.h"
#include "bmruntime_interface.h"
#include "common/ObjectMetadata.h"

namespace sophon_stream {
namespace element {
namespace decode {

/**
 * face transformations gpu process class
 */
class SophgoDecode {
 public:
  SophgoDecode();

  common::ErrorCode init(SophgoContext& context);
  /**
   * preprocess
   * @param[in] context: input and output config
   * @param[in] objectMetadatas: inputData
   * @return preprocess error code or common::ErrorCode::SUCCESS
   */
  common::ErrorCode process(
      SophgoContext& context,
      std::shared_ptr<common::ObjectMetadata>& objectMetadata);
  void uninit();

 private:
  std::string mStrError;
  bm_handle_t m_handle;
  VideoDecFFM decoder;
  // static std::map<int, std::shared_ptr<decode::gpu::FFmpegHwDevice>>
  // mSMapHwDevice; std::shared_ptr<decode::gpu::FFmpegFormatInput>
  // mSpFormatInput = nullptr; std::shared_ptr<decode::gpu::FFmpegDecoder>
  // mSpDecoder = nullptr;

  std::string mUrl;
  float mResizeRate = 1.0f;
  int mTimeout = 0;
  int mSourceType = 0;  // 0视频文件1是文件夹2是rtsp或rtmp
};

}  // namespace decode
}  // namespace element
}  // namespace sophon_stream
