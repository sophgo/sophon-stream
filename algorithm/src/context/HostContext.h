#pragma once

#include "../Context.h"
namespace sophon_stream{
namespace algorithm {
namespace context {

struct HostContext :public algorithm::Context {

    std::vector<float> result;
    common::ErrorCode init(const std::string& json) override;

};
} // namespace context
} // namespace algorithm
} // namespace sophon_stream
