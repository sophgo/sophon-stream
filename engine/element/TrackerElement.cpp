#include "TrackerElement.h"

#include <dlfcn.h>

#include <nlohmann/json.hpp>

#include "common/Logger.h"
#include "common/ObjectMetadata.h"
#include "framework/ElementFactory.h"

namespace sophon_stream {
namespace element {

TrackerElement::TrackerElement(){
    std::cout<<"TrackerElement construct!!!"<<std::endl;
}

TrackerElement::~TrackerElement() {
}

void TrackerElement::doSth() {

}

constexpr const char* JSON_MODELS_FIELD = "models";
constexpr const char* JSON_MODEL_NAME_FIELD = "name";
constexpr const char* JSON_MODEL_SHARED_OBJECT_FIELD = "shared_object";

common::ErrorCode TrackerElement::initInternal(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

    do {
        auto configure = nlohmann::json::parse(json, nullptr, false);
        if (!configure.is_object()) {
            IVS_ERROR("Parse json fail or json is not object, worker id: {0:d}, json: {0}",
                      getId(),
                      json);
            errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
            break;
        }

        mSpTrackerSort = std::make_shared<algorithm::tracker_sort::TrackerChannels>();
        errorCode = mSpTrackerSort->init(json);
        if (common::ErrorCode::SUCCESS != errorCode) {
            IVS_ERROR("tracker algorithm init failed, worker id: {0:d}, json: {1}",
                      getId(),
                      json);
            break;
        }

    } while (false);
    

    return errorCode;
}

void TrackerElement::uninitInternal() {
}

common::ErrorCode TrackerElement::doWork() {
    auto data = getData(0);
    if (!data) {
        popData(0);
        return common::ErrorCode::SUCCESS;
    }
    auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
    if(objectMetadata->mFrame && objectMetadata->mFrame->mEndOfStream){
        // IVS_CRITICAL("tracker element eof!!!");
        // return common::ErrorCode::STREAM_END;
    }
        
    // IVS_CRITICAL("tracker before process");
    mSpTrackerSort->process(objectMetadata,std::bind(&TrackerElement::putTask,this,std::placeholders::_1));
    // IVS_CRITICAL("tracker after process");
    popData(0);

    common::ErrorCode errorCode = sendData(0,
                                   std::static_pointer_cast<void>(objectMetadata),
                                   std::chrono::milliseconds(200));

    return common::ErrorCode::SUCCESS;
}

void TrackerElement::putTask(std::shared_ptr<common::ObjectMetadata>& data){
     IVS_CRITICAL("tracker element puttask");
        common::ErrorCode errorCode = sendData(0,
                                       std::static_pointer_cast<void>(data),
                                       std::chrono::milliseconds(200));
}


//static constexpr const char* NAME = "tracker_element";
REGISTER_WORKER("tracker_element", TrackerElement)
}
}
