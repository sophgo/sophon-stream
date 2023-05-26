//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-PIPELINE is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ENGINE_FRAMEWORK_DATAPIPE_H_
#define SOPHON_STREAM_ENGINE_FRAMEWORK_DATAPIPE_H_

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

#include "common/ErrorCode.h"
#include "common/Logger.h"

namespace sophon_stream {
namespace framework {

/**
 * A class buff data with FIFO.
 */
class DataPipe {
 public:
  using PushHandler = std::function<void()>;

  /**
   * Constructor of class DataPipe.
   */
  DataPipe();

  /**
   * Destructor of class DataPipe.
   */
  ~DataPipe();

  DataPipe(const DataPipe&) = delete;
  DataPipe& operator=(const DataPipe&) = delete;
  DataPipe(DataPipe&&) = default;
  DataPipe& operator=(DataPipe&&) = default;

  /**
   * Push data to this DataPipe.
   * @param[in] data : The data that will be push.
   * @param[in] timeout : The duration that will be wait for.
   * @return If timeout, it will return error,
   * otherwise return common::ErrorCode::SUCESS.
   */
  template <class Rep, class Period>
  common::ErrorCode pushData(
      std::shared_ptr<void> data,
      const std::chrono::duration<Rep, Period>& timeout) {
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
  std::shared_ptr<void> getData() const;

  /**
   * Pop next data of this DataPipe.
   */
  void popData();

  /**
   * Set push handler which will be call in pushData().
   */
  void setPushHandler(PushHandler pushHandler) { mPushHandler = pushHandler; }

  void setCapacity(std::size_t capacity) { mCapacity = capacity; }

  std::size_t getSize() const {
    std::lock_guard<std::mutex> lock(mDataQueueMutex);
    return mDataQueue.size();
  }

  std::size_t getCapacity() const { return mCapacity; }

 private:
  std::deque<std::shared_ptr<void> > mDataQueue;
  mutable std::mutex mDataQueueMutex;
  std::condition_variable mDataQueueCond;
  PushHandler mPushHandler;
  std::size_t mCapacity;
};

}  // namespace framework
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ENGINE_FRAMEWORK_DATAPIPE_H_
