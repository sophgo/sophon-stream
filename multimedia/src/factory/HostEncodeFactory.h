/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by xingzilong, 2020-04-27

**************************************************/

#ifndef LYNXI_IVS_ALGORITHM_HOST_ENCODE_FACTORY_H_
#define LYNXI_IVS_ALGORITHM_HOST_ENCODE_FACTORY_H_

#include "../AlgorithmFactory.h"
#include "../Context.h"
#include "../Inference.h"
#include "../PostProcess.h"
#include "../PreProcess.h"


namespace lynxi {
namespace ivs {
namespace algorithm {
namespace factory {

class HostEncodeFactory : public algorithm::AlgorithmFactory {
public:
    /**
     * 创建context
     * @return context对象
     */
    std::shared_ptr<algorithm::Context> makeContext() override;
    /**
     * 创建预处理
     * @return 预处理对象
     */
    std::shared_ptr<algorithm::PreProcess> makePreProcess() override;
    /**
     * 创建推理
     * @return 推理对象
     */
    std::shared_ptr<algorithm::Inference> makeInference() override;
    /**
     * 创建后处理
     * @return 后处理对象
     */
    std::shared_ptr<algorithm::PostProcess> makePostProcess() override;

};

} // namespace factory
} // namespace algorithm
} // namespace ivs
} // namespace lynxi

#endif // LYNXI_IVS_ALGORITHM_HOST_ENCODE_FACTORY_H_
