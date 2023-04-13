#pragma once

#include "../PreProcess.h"

namespace sophon_stream {
namespace algorithm {
namespace pre_process {

class YoloXPre : public algorithm::PreProcess {
    public:

    common::ErrorCode preProcess(algorithm::Context & context,
        common::ObjectMetadatas& objectMetadatas) override;
};

}
}
}