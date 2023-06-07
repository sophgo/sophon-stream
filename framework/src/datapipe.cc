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

DataPipe::DataPipe() : mCapacity(DEFAULT_DATA_PIPE_CAPACITY) {}

DataPipe::~DataPipe() {}

common::ErrorCode DataPipe::pushData(std::shared_ptr<void> data) {
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

std::shared_ptr<void> DataPipe::getData() const {
  std::lock_guard<std::mutex> lock(mDataQueueMutex);
  if (mDataQueue.empty()) {
    return std::shared_ptr<void>();
  } else {
    return mDataQueue.front();
  }
}

std::shared_ptr<void> DataPipe::popData()
{
  std::lock_guard<std::mutex> lock(mDataQueueMutex);
  std::shared_ptr<void> data = nullptr;
  if(!mDataQueue.empty())
  {
   data = mDataQueue.front();
   mDataQueue.pop_front(); 
  }
  return data;
}

void DataPipe::setPushHandler(PushHandler pushHandler) {
  mPushHandler = pushHandler;
}

void DataPipe::setCapacity(std::size_t capacity) { mCapacity = capacity; }

std::size_t DataPipe::getSize() const {
  std::lock_guard<std::mutex> lock(mDataQueueMutex);
  return mDataQueue.size();
}

std::size_t DataPipe::getCapacity() const { return mCapacity; }

}  // namespace framework
}  // namespace sophon_stream
