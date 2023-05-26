#pragma once

#include "../../../framework/element.h"
#include "sort.hpp"

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