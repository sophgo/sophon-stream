//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "connector.h"

namespace sophon_stream {
namespace framework {

Connector::Connector(int dataPipeCount) {
  mCapacity = dataPipeCount;
  mDataPipes.reserve(mCapacity);
  for (int i = 0; i < mCapacity; ++i) {
    auto datapipe = std::make_shared<DataPipe>();
    mDataPipes.push_back(datapipe);
  }
}

std::shared_ptr<void> Connector::popData(int id) {
  return getDataPipe(id)->popData();
}

common::ErrorCode Connector::pushData(
    int id, std::shared_ptr<void> data) {
  return getDataPipe(id)->pushData(data);
}


int Connector::getCapacity() const { return mCapacity; }

std::shared_ptr<DataPipe> Connector::getDataPipe(int id) const {
  if (id < 0 || id > mDataPipes.size()) {
    IVS_ERROR("Error DataPipe Id!");
    return nullptr;
  }
  return mDataPipes[id];
}

}  // namespace framework
}  // namespace sophon_stream