/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by Yang Jun <jun.yang@lynxi.com>, 20-3-6

**************************************************/

#ifndef LYNXI_IVS_ALGORITHM_CONTEXT_LYN_H_
#define LYNXI_IVS_ALGORITHM_CONTEXT_LYN_H_

#include "../Context.h"
#include "aiModelManagerWrapper.h" //neural network related APIs

namespace lynxi {
namespace ivs {
namespace algorithm {
namespace context {


struct LynContext :public Context {

    std::vector<TJ_OUTPUT_T*> result;
    TJ_MODEL_HANDLE_T* conModMngr;
    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    common::ErrorCode init(const std::string& json) override;
};
} // namespace context
} // namespace algorithm
} // namespace ivs
} // namespace lynxi

#endif // LYNXI_IVS_ALGORITHM_CONTEXT_LYN_H_


