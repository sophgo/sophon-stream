/**********************************************

 Copyright (c) 2018 LynxiTech Inc - All rights reserved.

 NOTICE: All information contained here is, and remains
 the property of LynxiTech Incorporation. This file can not
 be copied or distributed without permission of LynxiTech Inc.

 Author: written by Xing Zilong <zilong.xing@lynxi.com>, 20-5-12

**************************************************/

#ifndef LYNXI_IVS_ALGORITHM_CONTEXT_HOST_H_
#define LYNXI_IVS_ALGORITHM_CONTEXT_HOST_H_

#include "../Context.h"
#include "../pre_process/tracker/QualityControl.hpp"
namespace lynxi {
namespace ivs {
namespace algorithm {
namespace context {
struct TrackConfig{
    float mIou;
    int mMaxAge;
    int mMinhins;
    int mUpdateTimes;
};
struct QualityFilterConfig{
    int mTopn;
    int mBaseTime;
    std::shared_ptr<tracker_sort::QualityControl> mSpQualityControl;
};

struct HostContext :public algorithm::Context {

    std::vector<float> result;

    /**
     * context初始化
     * @param[in] json: 初始化的json字符串
     * @return 错误码
     */
    common::ErrorCode init(const std::string& json) override;
    TrackConfig mTrackConfig;
    QualityFilterConfig mQualityFilterConfig;
};
} // namespace context
} // namespace algorithm
} // namespace ivs
} // namespace lynxi

#endif // LYNXI_IVS_ALGORITHM_CONTEXT_HOST_H_

