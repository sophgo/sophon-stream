//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "element_factory.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "common/ErrorCode.h"
#include "common/Singleton.hpp"
#include "element.h"

namespace sophon_stream {
namespace framework {

/**
 * Element工厂
 */

/**
 * 添加Element产生器
 * @param[in] elementName:
 * @param[in] elementMaker:
 */
common::ErrorCode ElementFactory::addElementMaker(
    const std::string& elementName, ElementMaker elementMaker) {
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
 * 获取产生器
 * @param[in] elementName:
 */
std::shared_ptr<framework::Element> ElementFactory::make(
    const std::string& elementName) {
  auto elementMakerIt = mElementMakerMap.find(elementName);
  if (mElementMakerMap.end() != elementMakerIt && elementMakerIt->second) {
    return elementMakerIt->second();
  } else {
    IVS_ERROR("Can not find element maker, name: {0}", elementName);
    return std::shared_ptr<framework::Element>();
  }
}

/**
 * Constructor of class ElementFactory.
 */
ElementFactory::ElementFactory() {}

/**
 * Destructor of class ElementFactory.
 */
ElementFactory::~ElementFactory() {
  for (auto it = mElementMakerMap.begin(); it != mElementMakerMap.end();) {
    it = mElementMakerMap.erase(it);
  }
}

}  // namespace framework
}  // namespace sophon_stream
