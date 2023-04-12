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
                   ElementMaker elementMaker);
    /**
     * 移除Element产生器
     * @param[in] elementName:
     * @param[in] elementMaker:
     */
    void removeElementMaker(const std::string& elementName);

    /**
     * 获取产生器
     * @param[in] elementName:
     */
    std::shared_ptr<framework::Element> make(const std::string& elementName);

private:
    friend class common::Singleton<ElementFactory>;

    /**
     * Constructor of class ElementFactory.
     */
    ElementFactory();
    /**
     * Destructor of class ElementFactory.
     */
    ~ElementFactory();

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
