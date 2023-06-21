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
  std::unique_lock<std::mutex> lock(mDataQueueMutex);
  if(mDataQueue.size() < mCapacity) {
    mDataQueue.push_back(data);
    return common::ErrorCode::SUCCESS;
  }
  return common::ErrorCode::DATA_PIPE_FULL;
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

}  // namespace framework
}  // namespace sophon_stream
