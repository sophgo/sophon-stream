#pragma once 
#include <memory>

#include "../AlgorithmApi.h"
#include "Yolov5SophgoContext.h"
#include "Yolov5Inference.h"
#include "Yolov5Post.h"
#include "Yolov5Pre.h"

namespace sophon_stream {
namespace algorithm {

/**
 * 算法模块
 */
class Yolov5Algorithm : public algorithm::AlgorithmApi {
  public:
    /**
     * 构造函数
     */
    Yolov5Algorithm();
    /**
     * 析构函数
     */
    ~Yolov5Algorithm() override;

    Yolov5Algorithm(const Yolov5Algorithm&) = delete;
    Yolov5Algorithm& operator =(const Yolov5Algorithm&) = delete;
    Yolov5Algorithm(Yolov5Algorithm&&) = default;
    Yolov5Algorithm& operator =(Yolov5Algorithm&&) = default;

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


    // static constexpr const char* NAME = "yolov5_algorithm";

  private:
    std::shared_ptr<yolov5::Yolov5SophgoContext> mContext; //context对象
    std::shared_ptr<yolov5::Yolov5Pre> mPreProcess; //预处理对象
    std::shared_ptr<yolov5::Yolov5Inference> mInference; //推理对象
    std::shared_ptr<yolov5::Yolov5Post> mPostProcess; //后处理对象
    bool mAgency=false; //本地或者远程flag

    bool use_pre = false;
    bool use_infer = false;
    bool use_post = false;

};

} // namespace algorithm
} // namespace sophon_stream


