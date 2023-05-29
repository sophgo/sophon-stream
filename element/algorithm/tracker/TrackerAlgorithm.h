//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_TRACKER_ALGORITHM_H_
#define SOPHON_STREAM_ELEMENT_TRACKER_ALGORITHM_H_

#include "element.h"
#include "sort.h"

namespace sophon_stream {
namespace element {

class TrackerAlgorithm : public ::sophon_stream::framework::Element {
 public:
  TrackerAlgorithm();
  ~TrackerAlgorithm() override;

  TrackerAlgorithm(const TrackerAlgorithm&) = delete;
  TrackerAlgorithm& operator=(const TrackerAlgorithm&) = delete;
  TrackerAlgorithm(TrackerAlgorithm&&) = default;
  TrackerAlgorithm& operator=(TrackerAlgorithm&&) = default;

  common::ErrorCode initInternal(const std::string& json);
  void uninitInternal() override;

  common::ErrorCode doWork() override;
  void putTask(std::shared_ptr<common::ObjectMetadata>& data);

 private:
  std::shared_ptr<element::tracker_sort::TrackerChannels> mSpTrackerSort;
};

}  // namespace element
}  // namespace sophon_stream

#endif // SOPHON_STREAM_ELEMENT_TRACKER_ALGORITHM_H_