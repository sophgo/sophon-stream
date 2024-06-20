//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_DECODER_H_
#define SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_DECODER_H_

#include <dirent.h>

#include <opencv2/opencv.hpp>
#include <regex>
#include <string>
#include <unordered_map>

#include "channel.h"
#include "common/no_copyable.h"
#include "ff_decode.h"
#include "http_base64_mgr.h"

namespace sophon_stream {
namespace element {
namespace decode {

class Decoder : public ::sophon_stream::common::NoCopyable {
 public:
  Decoder();
  ~Decoder();

  common::ErrorCode init(int deviceId, const ChannelOperateRequest& request);
  common::ErrorCode process(
      std::shared_ptr<common::ObjectMetadata>& objectMetadata);
  void uninit();

 private:
  bm_handle_t m_handle;
  VideoDecFFM decoder;

  std::string mUrl;
  int mDeviceId;
  int mLoopNum;
  int mImgIndex;
  int mFrameCount;
  ChannelOperateRequest::SourceType mSourceType;
  std::vector<std::string> mImagePaths;
  HTTP_Base64_Mgr* mgr;
  bmcv_rect_t mRoi;
  bool mRoiPredefined = false;

  double mFps;
  int mSampleInterval;
  ChannelOperateRequest::SampleStrategy mSampleStrategy;

  // camera synchronization
  static std::mutex decoder_mutex;
  static std::condition_variable decoder_cv;
  static int numThreadsReady;
  static std::atomic<int> numThreadsTotal;
};
}  // namespace decode
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_MULTIMEDIA_DECODE_DECODER_H_