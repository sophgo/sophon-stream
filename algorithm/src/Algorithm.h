#pragma once 
#include <memory>

#include "AlgorithmApi.h"
#include "Context.h"
#include "Inference.h"
#include "PostProcess.h"
#include "PreProcess.h"

namespace sophon_stream {
namespace algorithm {

/**
 * 算法模块
 */
class Algorithm : public algorithm::AlgorithmApi {
  public:
    /**
     * 构造函数
     */
    Algorithm();
    /**
     * 析构函数
     */
    ~Algorithm() override;

    Algorithm(const Algorithm&) = delete;
    Algorithm& operator =(const Algorithm&) = delete;
    Algorithm(Algorithm&&) = default;
    Algorithm& operator =(Algorithm&&) = default;

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
    void process(common::ObjectMetadatas& objectMetadatas) override;
    /**
     * 资源释放函数
     */
    void uninit() override;

  private:
    std::shared_ptr<Context> mContext; //context对象
    std::shared_ptr<PreProcess> mPreProcess; //预处理对象
    std::shared_ptr<Inference> mInference; //推理对象
    std::shared_ptr<PostProcess> mPostProcess; //后处理对象
    bool mAgency=false; //本地或者远程flag

};

} // namespace algorithm
} // namespace sophon_stream


