/**********************************************

© 2018 北京灵汐科技有限公司 版权所有。
* 注意：以下内容均为北京灵汐科技有限公司原创，未经本公司允许，不得转载，否则将视
为侵权；
* 对于不遵守此声明或者其他违法使用以下内容者，本公司依法保留追究权。

© 2018 Lynxi Technologies Co., Ltd. All rights reserved.
* NOTICE: All information contained here is, and remains the property of Lynxi.
* This file can not be copied or distributed without the permission of Lynxi Technologies Co., Ltd.

 Author: zilong.xing 2020-04-27

 Content: Decode GPU

**************************************************/
#ifndef DECODE_GPU_PRE_H
#define DECODE_GPU_PRE_H

#include "../PreProcess.h"

namespace lynxi {
namespace ivs {
namespace algorithm {
namespace pre_process {

/**
 * face transformations gpu process class
 */
class HostEncodePre : public algorithm::PreProcess {
  public :

    /**
     * preprocess
     * @param[in] context: input and output config
     * @param[in] objectMetadatas: inputData
     * @return preprocess error code or common::ErrorCode::SUCCESS
     */
    common::ErrorCode preProcess(algorithm::Context& context,
                                 common::ObjectMetadatas& objectMetadatas) override;


};

} // namespace pre_process
} // namespace algorithm
} // namespace ivs
} // namespace lynxi

#endif //DECODE_GPU_PRE_H
