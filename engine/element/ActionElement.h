#pragma once

#include "algorithm/src/AlgorithmApi.h"
#include "framework/ElementNew.h"

namespace sophon_stream {
namespace element {
/**
 * @brief ActionElement是一个通用的element, 用于处理通用算法
 */
class ActionElement : public framework::Element {
  public:
    ActionElement();
    ~ActionElement() override;

    ActionElement(const ActionElement&) = delete;
    ActionElement& operator =(const ActionElement&) = delete;
    ActionElement(ActionElement&&) = default;
    ActionElement& operator =(ActionElement&&) = default;

    static void doSth();

  private:
    bool hasEof = false;

    common::ErrorCode initInternal(const std::string& json) override;
    void uninitInternal() override;

    common::ErrorCode doWork() override;
    /**
     * @brief 从processed objectmetadatas队列中获取数据并发送至0通道
     */
    common::ErrorCode sendProcessedData();

    /**
     * @brief 需要推理的batch数
     */
    int mBatch;

    /**
     * @brief 要加载的so句柄集合 
     */
    std::vector<std::shared_ptr<void> > mSharedObjectHandles;

    /**
     * @brief 算法api对象的集合 
     */
    std::vector<std::pair<std::string/* name */,
        std::shared_ptr<algorithm::AlgorithmApi> > > mAlgorithmApis;

    /**
     * @brief 用于多线程处理时，数据互斥
     */
    std::mutex mMutex;

    /**
     * @brief 上一次推理前datapipe中dataqueue的size 
     */
    int mLastDataCount;

    /**
     * @brief 发送数据缓冲
     */
    std::deque<std::shared_ptr<common::ObjectMetadata> > mProcessedObjectMetadatas;

    /**
     * @brief batch数据缓冲
     */
    common::ObjectMetadatas mPendingObjectMetadatas;
};

} // namespace element
} // namespace sophon_stream

