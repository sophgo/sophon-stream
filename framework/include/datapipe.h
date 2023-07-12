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

  /**
   * @brief 从队首弹出数据
   * @return std::shared_ptr<void> 若队列非空则弹出队首，队列为空返回nullptr
   */
  std::shared_ptr<void> popData();

  /**
   * @brief 向队列末尾push数据
   * @return common::ErrorCode
   * 成功返回common::ErrorCode::SUCCESS，失败返回common::ErrorCode::DATA_PIPE_FULL
   */
  common::ErrorCode pushData(std::shared_ptr<void> data);

  int getSize();

 private:
  std::deque<std::shared_ptr<void> > mDataQueue;
  mutable std::mutex mDataQueueMutex;
  std::size_t mCapacity;

  const std::chrono::milliseconds timeout{200};
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_DATAPIPE_H_
