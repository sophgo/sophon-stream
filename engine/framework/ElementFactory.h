#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "common/ErrorCode.h"
#include "common/Singleton.hpp"

#include "ElementNew.h"

namespace sophon_stream {
namespace framework {

/**
 * Element工厂
 */
class ElementFactory {
public:
    using ElementMaker = std::function<std::shared_ptr<framework::Element>()>;

    /**
     * 添加Element产生器
     * @param[in] elementName:
     * @param[in] elementMaker:
     */
    common::ErrorCode
    addElementMaker(const std::string& elementName, 
                   ElementMaker elementMaker) {
    auto elementMakerIt = mElementMakerMap.find(elementName);
    std::cout << "current element added:" << elementName << std::endl;
    if (mElementMakerMap.end() != elementMakerIt) {
        IVS_ERROR("Repeated element name, name: {0}", elementName);
        return common::ErrorCode::REPEATED_WORKER_NAME;
    }

    mElementMakerMap[elementName] = elementMaker;

    return common::ErrorCode::SUCCESS;
}
    /**
     * 移除Element产生器
     * @param[in] elementName:
     * @param[in] elementMaker:
     */
    // void removeElementMaker(const std::string& elementName);

    /**
     * 获取产生器
     * @param[in] elementName:
     */
    std::shared_ptr<framework::Element> make(const std::string& elementName){
    auto elementMakerIt = mElementMakerMap.find(elementName);
    if (mElementMakerMap.end() != elementMakerIt
            && elementMakerIt->second) {
        return elementMakerIt->second();
    } else {
        IVS_ERROR("Can not find element maker, name: {0}", elementName);
        return std::shared_ptr<framework::Element>();
    }
    }

private:
    friend class common::Singleton<ElementFactory>;

    /**
     * Constructor of class ElementFactory.
     */
    ElementFactory() {}
    /**
     * Destructor of class ElementFactory.
     */
    ~ElementFactory() {}

    ElementFactory(const ElementFactory&) = delete;
    ElementFactory& operator =(const ElementFactory&) = delete;
    ElementFactory(ElementFactory&&) = delete;
    ElementFactory& operator =(ElementFactory&&) = delete;

    std::map<std::string, ElementMaker> mElementMakerMap;
};

using SingletonElementFactory = common::Singleton<ElementFactory>;

#define REGISTER_WORKER(elementName, ElementClass) \
    struct ElementClass##Register { \
        ElementClass##Register() { \
            auto& elementFactory = ::sophon_stream::framework::SingletonElementFactory::getInstance(); \
            std::cout<<elementName<<std::endl; \
            elementFactory.addElementMaker(elementName, []() { \
                                             return std::make_shared<ElementClass>(); \
                                         }); \
        } \
    }; \
    static ElementClass##Register g##ElementClass##Register;

} // namespace framework
} // namespace sophon_stream
