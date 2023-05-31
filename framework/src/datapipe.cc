//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "datapipe.h"

namespace sophon_stream {
namespace framework {

#define DEFAULT_DATA_PIPE_CAPACITY 20

/**
 * Constructor of class DataPipe.
 */
DataPipe::DataPipe() : mCapacity(DEFAULT_DATA_PIPE_CAPACITY) {}

/**
 * Destructor of class DataPipe.
 */
DataPipe::~DataPipe() {}

common::ErrorCode DataPipe::pushData(std::shared_ptr<void> data,
                                     const std::chrono::milliseconds& timeout) {
  bool noTimeout = true;
  std::unique_lock<std::mutex> lock(mDataQueueMutex);
  noTimeout = mDataQueueCond.wait_for(
      lock, timeout, [this]() { return mDataQueue.size() < mCapacity; });
  if (noTimeout) {
    mDataQueue.push_back(data);
    if (mDataQueue.size() >= mCapacity) {
      IVS_WARN("data queue size too high, size is :{0}", mDataQueue.size());
    }
    if (mPushHandler) {
      mPushHandler();
    }

    return common::ErrorCode::SUCCESS;
  } else {
    IVS_WARN("Push data timeout");
    return common::ErrorCode::TIMEOUT;
  }
}

/**
 * Get next data of this DataPipe.
 * @return Return data.
 */
std::shared_ptr<void> DataPipe::getData() const {
  std::lock_guard<std::mutex> lock(mDataQueueMutex);
  if (mDataQueue.empty()) {
    return std::shared_ptr<void>();
  } else {
    return mDataQueue.front();
  }
}

/**
 * Pop next data of this DataPipe.
 */
void DataPipe::popData() {
  {
    std::lock_guard<std::mutex> lock(mDataQueueMutex);
    if (mDataQueue.empty()) {
      return;
    }
    mDataQueue.pop_front();
  }

  mDataQueueCond.notify_one();
}

}  // namespace framework
}  // namespace sophon_stream
