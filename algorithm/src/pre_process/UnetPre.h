#pragma once

#include "../PreProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

class UnetPre : public algorithm::PreProcess {
    public:
        /**
         * execute preprocess
         * @param[in] objectMetadatas: input data
         * @param[out] context: data transport to model
         * @return ErrorCode
        */
        common::ErrorCode preProcess(algorithm::Context & context,
                                    common::ObjectMetadatas & objectMetadatas) override;
        float get_aspect_scaled_ratio(int src_w, int src_h, int dst_w, int dst_h, bool *pIsAligWidth);
};
}
}
}