//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_CONNECTOR_H_
#define SOPHON_STREAM_FRAMEWORK_CONNECTOR_H_

#include "common/no_copyable.h"
#include "datapipe.h"

namespace sophon_stream {
namespace framework {

class Connector : public ::sophon_stream::common::NoCopyable {
 public:
  Connector(int dataPipeCount);

  std::shared_ptr<void> popData(int id);
  common::ErrorCode pushData(int id, std::shared_ptr<void> data);
  /**
   * @brief 获取Connector中dataPipe的数量
   * @return int 当前Connector中dataPipe数量
   */
  int getCapacity() const;

  std::shared_ptr<DataPipe> getDataPipe(int id) const;


 private:
  std::vector<std::shared_ptr<DataPipe>> mDataPipes;
  int mCapacity = 0;
};

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_CONNECTOR_H_