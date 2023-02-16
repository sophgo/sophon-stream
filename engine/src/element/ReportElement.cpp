#include "ReportElement.h"

#include <dlfcn.h>

#include <nlohmann/json.hpp>

#include "common/Logger.h"
#include "common/ObjectMetadata.h"
#include "framework/ElementFactory.h"

namespace sophon_stream {
namespace element {

ReportElement::ReportElement() {}
ReportElement::~ReportElement() {}

void ReportElement::doSth() {
}

common::ErrorCode ReportElement::initInternal(const std::string& json) {
    return common::ErrorCode::SUCCESS;
}

void ReportElement::uninitInternal() {}

void ReportElement::onStart() {
    IVS_INFO("ReportElement start...");
}
void ReportElement::onStop() {
    IVS_INFO("ReportElement stop...");
}
common::ErrorCode ReportElement::doWork() {
    auto data = getData(0);
    popData(0);

    IVS_DEBUG("report dowork! before!");
    common::ErrorCode ret = sendData(0, data, std::chrono::seconds(4));
    if(ret!=common::ErrorCode::SUCCESS) IVS_ERROR("ret:{0}", (int)ret);
    IVS_DEBUG("report dowork!");
    return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("report_element", ReportElement)

} // namespace framework
} // namespace sophon_stream
