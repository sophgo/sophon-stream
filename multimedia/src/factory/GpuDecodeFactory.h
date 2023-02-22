/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by xingzilong, 2020-04-27

**************************************************/

#ifndef LYNXI_IVS_MULTIMEDIA_GPU_DECODE_FACTORY_H_
#define LYNXI_IVS_MULTIMEDIA_GPU_DECODE_FACTORY_H_

#include "../MultiMediaFactory.h"
#include "../Context.h"
#include "../Process.h"


namespace lynxi {
namespace ivs {
namespace multimedia {
namespace factory {

class GpuDecodeFactory : public multimedia::MultiMediaFactory {
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
} // namespace multimedia
} // namespace ivs
} // namespace lynxi

#endif // LYNXI_IVS_ALGORITHM_GPU_DECODE_FACTORY_H_

