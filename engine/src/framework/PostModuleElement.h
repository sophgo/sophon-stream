#pragma once

#include <set>

#include "common/ObjectMetadata.h"

#include "ElementNew.h"

namespace sophon_stream {
namespace framework {

class PostModuleElement : public framework::Element {
public:
    /**
     * Constructor of class PreModuleElement.
     */
    PostModuleElement();
    /**
     * Destructor of class PreModuleElement.
     */
    ~PostModuleElement() override;

    PostModuleElement(const PostModuleElement&) = delete;
    PostModuleElement& operator =(const PostModuleElement&) = delete;
    PostModuleElement(PostModuleElement&&) = default;
    PostModuleElement& operator =(PostModuleElement&&) = default;

    static constexpr int BEGIN_WORKER_ID = 2000;
    static int gCurrentElementId;

    static constexpr const char* NAME = "post_process_element";
    static constexpr const char* JSON_MODULE_COUNT_FIELD = "module_count";

private:
    /**
     * 执行初始化.
     * @param[in] json:
     * @return 错误码.
     */
    common::ErrorCode initInternal(const std::string& json) override;
    /**
     * 执行销毁.
     */
    void uninitInternal() override;

    /**
     * When push data to input port of this Element, will call this function in run(). 
     * Each push data will cause one success call.
     * If milliseconds timeout is not 0, timeout will also call this function, 
     * and if repeated timeout is true, this function will be call repeated with repeated timeout. 
     */
    common::ErrorCode doWork() override;

    std::set<std::shared_ptr<common::ObjectMetadata> > mSubObjectMetadataSet;
    std::size_t mModuleCount;
};

} // namespace framework
} // namespace sophon_stream

