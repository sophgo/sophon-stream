#pragma once

#include "../../../framework/ElementNew.h"
#include "sort.hpp"

namespace sophon_stream {
namespace algorithm {

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
  std::shared_ptr<algorithm::tracker_sort::TrackerChannels> mSpTrackerSort;
};

}  // namespace algorithm
}  // namespace sophon_stream