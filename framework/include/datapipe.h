//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_DATAPIPE_H_
#define SOPHON_STREAM_FRAMEWORK_DATAPIPE_H_

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

#include "common/error_code.h"
#include "common/logger.h"
#include "common/no_copyable.h"

namespace sophon_stream {
namespace framework {

class DataPipe : public ::sophon_stream::common::NoCopyable {
 public:
  using PushHandler = std::function<void()>;

  DataPipe();

  ~DataPipe();

  std::shared_ptr<void> popData();

  common::ErrorCode pushData(std::shared_ptr<void> data);

  void setPushHandler(PushHandler pushHandler);

  std::size_t getSize() const;

  std::size_t getCapacity() const;

 private:
  std::deque<std::shared_ptr<void> > mDataQueue;
  mutable std::mutex mDataQueueMutex;
  std::condition_variable mDataQueueCond;
  PushHandler mPushHandler;
  std::size_t mCapacity;

  const std::chrono::milliseconds timeout{200};
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_DATAPIPE_H_
