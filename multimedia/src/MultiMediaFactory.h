#pragma once

#include <memory>

#include "Context.h"
#include "Process.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 多媒体工厂
 */
class MultiMediaFactory {
public:
    /**
     * 创建context
     * @return context对象
     */
    virtual std::shared_ptr<multimedia::Context> makeContext() = 0;
    /**
     * 创建Process
     * @return Process对象
     */
    virtual std::shared_ptr<multimedia::Process> makeProcess() = 0;
};

} // namespace multimedia
} // namespace sophon_stream

