#pragma once

#include "common/ObjectMetadata.h"
#include <string>
#include <vector>
#include <memory>
#include "common/ErrorCode.h"
#include "UnetSophgoContext.h"

namespace sophon_stream {
namespace algorithm {
namespace unet {

class UnetPre {
    public:
        /**
         * execute preprocess
         * @param[in] objectMetadatas: input data
         * @param[out] context: data transport to model
         * @return ErrorCode
        */
        common::ErrorCode preProcess(UnetSophgoContext & context,
                                    common::ObjectMetadatas & objectMetadatas);
        float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *pIsAligWidth);
};
}
}
}