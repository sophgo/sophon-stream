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

#include "../Process.h"
#include "../process/decode/gpu/FFmpegFormatInput-new.h"
#include "../process/decode/gpu/FFmpegDecoder-new.h"

namespace lynxi {
namespace ivs {
namespace multimedia {
namespace process {

/**
 * face transformations gpu process class
 */
class GpuDecode : public multimedia::Process {
  public :
    GpuDecode();

    common::ErrorCode init(multimedia::Context& context);
    /**
     * preprocess
     * @param[in] context: input and output config
     * @param[in] objectMetadatas: inputData
     * @return preprocess error code or common::ErrorCode::SUCCESS
     */
    common::ErrorCode process(multimedia::Context& context,
                                 std::shared_ptr<common::ObjectMetadata>& objectMetadata) override;
    void uninit();

  private:
    std::string mStrError;
    static std::map<int, std::shared_ptr<decode::gpu::FFmpegHwDevice>> mSMapHwDevice;
    std::shared_ptr<decode::gpu::FFmpegFormatInput> mSpFormatInput = nullptr;
    std::shared_ptr<decode::gpu::FFmpegDecoder> mSpDecoder = nullptr;

    std::string mUrl;
    float mResizeRate = 1.0f;
    int mTimeout = 0;
    int mSourceType = 0;//0视频文件1是文件夹2是rtsp或rtmp

};

} // namespace process
} // namespace multimedia
} // namespace ivs
} // namespace lynxi

#endif //DECODE_GPU_PRE_H
