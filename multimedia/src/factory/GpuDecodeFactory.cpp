/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by zilong.xing, 2020-04-27

**************************************************/

#include "GpuDecodeFactory.h"
#include "../MultiMediaFactorySelector.h"
#include "../process/GpuDecode.h"
#include "../context/GpuContext.h"

namespace lynxi {
namespace ivs {
namespace multimedia {
namespace factory {

/**
 * 创建context
 * @return context对象
 */
std::shared_ptr<multimedia::Context> GpuDecodeFactory::makeContext() {
    return std::static_pointer_cast<multimedia::Context>(std::make_shared<context::GpuContext>());
}

/**
 * 创建Process
 * @return Process对象
 */
std::shared_ptr<multimedia::Process> GpuDecodeFactory::makeProcess() {
    return std::static_pointer_cast<multimedia::Process>(std::make_shared<process::GpuDecode>());
}

REGISTER_MULTIMEDIA_FACTORY("nvidia", "decode_picture", GpuDecodeFactory);

} // namespace factory
} // namespace multimedia
} // namespace ivs
} // namespace lynxi
