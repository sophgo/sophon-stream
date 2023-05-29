//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "TrackerAlgorithm.h"

#include <nlohmann/json.hpp>

#include "element_factory.h"
#include "common/logger.h"

namespace sophon_stream {
namespace element {

constexpr const char* JSON_BATCH_FIELD = "batch";

TrackerAlgorithm::TrackerAlgorithm() {
  std::cout << "TrackerAlgorithm construct!!!" << std::endl;
}

TrackerAlgorithm::~TrackerAlgorithm() {}

common::ErrorCode TrackerAlgorithm::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      IVS_ERROR(
          "Parse json fail or json is not object, worker id: {0:d}, json: {0}",
          getId(), json);
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    mSpTrackerSort =
        std::make_shared<element::tracker_sort::TrackerChannels>();
    errorCode = mSpTrackerSort->init(json);
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_ERROR("tracker algorithm init failed, worker id: {0:d}, json: {1}",
                getId(), json);
      break;
    }
  } while (false);
  return errorCode;
}

void TrackerAlgorithm::uninitInternal() {}

common::ErrorCode TrackerAlgorithm::doWork() {
  auto data = getData(0);
  if (!data) {
    popData(0);
    return common::ErrorCode::SUCCESS;
  }
  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  mSpTrackerSort->process(
      objectMetadata,
      std::bind(&TrackerAlgorithm::putTask, this, std::placeholders::_1));
  popData(0);

  common::ErrorCode errorCode =
      sendData(0, std::static_pointer_cast<void>(objectMetadata),
               std::chrono::milliseconds(200));

  return common::ErrorCode::SUCCESS;
}

void TrackerAlgorithm::putTask(std::shared_ptr<common::ObjectMetadata>& data) {
  common::ErrorCode errorCode = sendData(
      0, std::static_pointer_cast<void>(data), std::chrono::milliseconds(200));
}

REGISTER_WORKER("Tracker", TrackerAlgorithm)

}  // namespace element
}  // namespace sophon_stream