
#include "ElementFactory.h"

#include "common/Logger.h"

namespace sophon_stream {
namespace framework {

/**
 * 添加Element产生器
 * @param[in] elementName:
 * @param[in] elementMaker:
 */
common::ErrorCode
ElementFactory::addElementMaker(const std::string& elementName, 
                              ElementMaker elementMaker) {
    auto elementMakerIt = mElementMakerMap.find(elementName);
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
void ElementFactory::removeElementMaker(const std::string& elementName) {
    mElementMakerMap.erase(elementName);
}

/**
 * 获取产生器
 * @param[in] elementName:
 */
std::shared_ptr<framework::Element> ElementFactory::make(const std::string& elementName) {
    auto elementMakerIt = mElementMakerMap.find(elementName);
    if (mElementMakerMap.end() != elementMakerIt
            && elementMakerIt->second) {
        return elementMakerIt->second();
    } else {
        IVS_ERROR("Can not find element maker, name: {0}", elementName);
        return std::shared_ptr<framework::Element>();
    }
}

/**
 * Constructor of class ElementFactory.
 */
ElementFactory::ElementFactory() {
}

/**
 * Destructor of class ElementFactory.
 */
ElementFactory::~ElementFactory() {
}

} // namespace framework
} // namespace sophon_stream
