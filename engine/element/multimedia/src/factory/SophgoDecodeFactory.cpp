
#include "SophgoDecodeFactory.h"
#include "../MultiMediaFactorySelector.h"
#include "../process/SophgoDecode.h"
#include "../context/SophgoContext.h"

namespace sophon_stream {
namespace multimedia {
namespace factory {

/**
 * 创建context
 * @return context对象
 */
std::shared_ptr<multimedia::Context> SophgoDecodeFactory::makeContext() {
    return std::static_pointer_cast<multimedia::Context>(std::make_shared<context::SophgoContext>());
}

/**
 * 创建Process
 * @return Process对象
 */
std::shared_ptr<multimedia::Process> SophgoDecodeFactory::makeProcess() {
    return std::static_pointer_cast<multimedia::Process>(std::make_shared<process::SophgoDecode>());
}

REGISTER_MULTIMEDIA_FACTORY("sophgo", "decode_picture", SophgoDecodeFactory);

} // namespace factory
} // namespace algorithm
} // namespace sophon_stream
