#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "bmcv_api.h"
#include "bmcv_api_ext.h"
#include "bmlib_runtime.h"
#include "bmruntime_interface.h"
#include "common/bmnn_utils.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/no_copyable.h"
#include "common/object_metadata.h"
#include "ff_decode.h"

namespace sophon_stream {
namespace element {
namespace decode {

class Decoder :public ::sophon_stream::common::NoCopyable {
 public:
  Decoder();
  ~Decoder();

  common::ErrorCode init(int deviceId, const std::string& json);
  common::ErrorCode process(
      std::shared_ptr<common::ObjectMetadata>& objectMetadata);
  void uninit();

  static constexpr const char* JSON_URL = "url";

 private:
  bm_handle_t m_handle;
  VideoDecFFM decoder;

  std::string mUrl;
  int mDeviceId;
};
}  // namespace decode
}  // namespace element
}  // namespace sophon_stream
