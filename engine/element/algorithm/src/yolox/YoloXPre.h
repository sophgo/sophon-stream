#pragma once

#include "common/ObjectMetadata.h"
#include <string>
#include <vector>
#include <memory>
#include "common/ErrorCode.h"
#include "YoloXSophgoContext.h"



namespace sophon_stream {
namespace algorithm {
namespace yolox {

class YoloXPre {
    public:

    common::ErrorCode preProcess(YoloXSophgoContext & context,
        common::ObjectMetadatas& objectMetadatas);
};

}
}
}