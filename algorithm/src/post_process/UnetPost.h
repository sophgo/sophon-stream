#pragma once

#include "../PostProcess.h"
#include "../context/SophgoContext.h"

namespace sophon_stream{
namespace algorithm {
namespace post_process {

class UnetPost : public algorithm::PostProcess {
    public:
        void init(algorithm::Context& context) override;

        float sigmoid(float x);

        void postProcess(algorithm::Context& context,
                        common::ObjectMetadatas & objectMetadatas) override;

        std::shared_ptr<common::Frame> bm_image2Frame(bm_handle_t&& handle,bm_image & img);
};
}
}
}