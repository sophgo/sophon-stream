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
