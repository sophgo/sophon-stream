#pragma once

#include "framework/ElementNew.h"
#include "algorithm/tracker/sort.hpp"

namespace sophon_stream {
namespace element {

class TrackerElement : public framework::Element {
  public:
    TrackerElement();
    ~TrackerElement() override;

    TrackerElement(const TrackerElement&) = delete;
    TrackerElement& operator =(const TrackerElement&) = delete;
    TrackerElement(TrackerElement&&) = default;
    TrackerElement& operator =(TrackerElement&&) = default;

    static void doSth();

  private:
    common::ErrorCode initInternal(const std::string& json) override;
    void uninitInternal() override;

    common::ErrorCode doWork() override;

    void putTask(std::shared_ptr<common::ObjectMetadata>& data);
  private:
    std::shared_ptr<algorithm::tracker_sort::TrackerChannels> mSpTrackerSort;
};

}
}