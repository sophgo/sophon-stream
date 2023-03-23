#pragma once

#include "../MultiMediaFactory.h"
#include "../Context.h"
#include "../Process.h"

namespace sophon_stream {
namespace multimedia {
namespace factory {

class SophgoDecodeFactory : public multimedia::MultiMediaFactory {
public:
    /**
     * 创建context
     * @return context对象
     */
    std::shared_ptr<multimedia::Context> makeContext() override;
    /**
     * 创建Process
     * @return Process对象
     */
    std::shared_ptr<multimedia::Process> makeProcess() override;

};

} // namespace factory
} // namespace algorithm
} // namespace sophon_stream
