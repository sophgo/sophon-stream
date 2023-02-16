#pragma once

#include "framework/ElementNew.h"

namespace sophon_stream {
namespace element {

class ReportElement:public framework::Element {
  public:
    ReportElement();
    ~ReportElement() override;

    ReportElement(const ReportElement&) = delete;
    ReportElement& operator =(const ReportElement&) = delete;
    ReportElement(ReportElement&&) = default;
    ReportElement& operator =(ReportElement&&) = default;

    static void doSth();

  private:

    common::ErrorCode initInternal(const std::string& json) override;
    void uninitInternal() override;
    void onStart() override;
    void onStop()override;
    common::ErrorCode doWork() override;

};

} // namespace element
} // namespace sophon_stream

