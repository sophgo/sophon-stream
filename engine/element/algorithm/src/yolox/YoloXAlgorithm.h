#pragma once 
#include <memory>

#include "../AlgorithmApi.h"
#include "YoloXSophgoContext.h"
#include "YoloXInference.h"
#include "YoloXPost.h"
#include "YoloXPre.h"

namespace sophon_stream {
namespace algorithm {

/**
 * 算法模块
 */
class YoloXAlgorithm : public algorithm::AlgorithmApi {
  public:
    /**
     * 构造函数
     */
    YoloXAlgorithm();
    /**
     * 析构函数
     */
    ~YoloXAlgorithm() override;

    YoloXAlgorithm(const YoloXAlgorithm&) = delete;
    YoloXAlgorithm& operator =(const YoloXAlgorithm&) = delete;
    YoloXAlgorithm(YoloXAlgorithm&&) = default;
    YoloXAlgorithm& operator =(YoloXAlgorithm&&) = default;

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
    std::shared_ptr<yolox::YoloXSophgoContext> mContext; //context对象
    std::shared_ptr<yolox::YoloXPre> mPreProcess; //预处理对象
    std::shared_ptr<yolox::YoloXInference> mInference; //推理对象
    std::shared_ptr<yolox::YoloXPost> mPostProcess; //后处理对象
    bool mAgency=false; //本地或者远程flag

    bool use_pre = false;
    bool use_infer = false;
    bool use_post = false;

};

} // namespace algorithm
} // namespace sophon_stream


