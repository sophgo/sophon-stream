#pragma once

#include <memory>

#include "MultiMediaApi.h"
#include "Context.h"
#include "Process.h"

namespace sophon_stream {
namespace multimedia {

/**
 * 多媒体模块
 */
class MultiMedia : public multimedia::MultiMediaApi {
  public:
    /**
     * 构造函数
     */
    MultiMedia();
    /**
     * 析构函数
     */
    ~MultiMedia() override;

    MultiMedia(const MultiMedia&) = delete;
    MultiMedia& operator =(const MultiMedia&) = delete;
    MultiMedia(MultiMedia&&) = default;
    MultiMedia& operator =(MultiMedia&&) = default;

    /**
     * 初始化函数
     * @param[in] side:  设备类型
     * @param[in] deviceId:  设备ID
     * @param[in] json:  初始化字符串
     * @return 错误码
     */
    common::ErrorCode init(const std::string& side,
                           int deviceId,
                           const std::string& json) override;
    /**
     * 预测函数
     * @param[in/out] objectMetadatas:  输入数据和预测结果
     */
    common::ErrorCode process(std::shared_ptr<common::ObjectMetadata>& objectMetadata) override;
    /**
     * 资源释放函数
     */
    void uninit() override;

  private:
    std::shared_ptr<Context> mContext;
    std::shared_ptr<Process> mProcess;

};

} // namespace multimedia
} // namespace sophon_stream


