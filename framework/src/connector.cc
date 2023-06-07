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
  mDataPipeCount = dataPipeCount;
  mDataPipes.reserve(mDataPipeCount);
  for (int i = 0; i < mDataPipeCount; ++i) {
    auto datapipe = std::make_shared<DataPipe>();
    mDataPipes.push_back(datapipe);
  }
  mCapacity = mDataPipes[0] == nullptr ? mDataPipes[0]->getCapacity() : 0;
}

std::shared_ptr<void> Connector::popDataWithId(int id) {
  return getDataPipe(id)->popData();
}

common::ErrorCode Connector::pushDataWithId(
    int id, std::shared_ptr<void> data) {
  return getDataPipe(id)->pushData(data);
}

int Connector::getDataCount(int id) { return getDataPipe(id)->getSize(); }

int Connector::getDataPipeCount() const { return mDataPipeCount; }

int Connector::getCapacity() const { return mCapacity; }

int Connector::getDataPipeSize(int id) { return mDataPipes[id]->getSize(); }

std::shared_ptr<DataPipe> Connector::getDataPipe(int id) const {
  if (id < 0 || id > mDataPipes.size()) {
    IVS_ERROR("Error DataPipe Id!");
    return nullptr;
  }
  return mDataPipes[id];
}

}  // namespace framework
}  // namespace sophon_stream