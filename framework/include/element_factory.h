//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_ELEMENT_FACTORY_H_
#define SOPHON_STREAM_FRAMEWORK_ELEMENT_FACTORY_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "common/error_code.h"
#include "common/no_copyable.h"
#include "common/singleton.h"
#include "element.h"
//#include "group.h"

namespace sophon_stream {
namespace framework {

class ElementFactory : public ::sophon_stream::common::NoCopyable {
 public:
  using ElementMaker = std::function<std::shared_ptr<framework::Element>()>;

  /**
   * @brief 添加一个名为elementName的ElementMaker
   * @param elementName element名，需要和注册的名称一致
   * @param elementMaker 类型同ElementMaker的函数对象
   * @return common::ErrorCode
   * 成功返回common::ErrorCode::SUCCESS，element名称重复返回common::ErrorCode::REPEATED_WORKER_NAME
   */
  common::ErrorCode addElementMaker(const std::string& elementName,
                                    ElementMaker elementMaker);

  /**
   * @brief 根据elementName查找并调用对应的ElementMaker，生产element
   * @param elementName element名，需要和注册的名称一致
   * @return std::shared_ptr<framework::Element> element智能指针
   */
  std::shared_ptr<framework::Element> make(const std::string& elementName);

  friend class common::Singleton<ElementFactory>;

  ElementFactory();

  std::map<std::string, ElementMaker> mElementMakerMap;

  ~ElementFactory();
};

using SingletonElementFactory = common::Singleton<ElementFactory>;

/**
 * @brief 向工厂中注册element
 */
#define REGISTER_WORKER(elementName, ElementClass)                            \
  struct ElementClass##Register {                                             \
    ElementClass##Register() {                                                \
      auto& elementFactory =                                                  \
          ::sophon_stream::framework::SingletonElementFactory::getInstance(); \
      std::cout << elementName << std::endl;                                  \
      elementFactory.addElementMaker(                                         \
          elementName, []() { return std::make_shared<ElementClass>(); });    \
    }                                                                         \
  };                                                                          \
  static ElementClass##Register g##ElementClass##Register;

#define REGISTER_TEMPLATE_WORKER(elementName, GroupElement, ElementClass)     \
  struct Group##ElementClass##Register {                                      \
    Group##ElementClass##Register() {                                         \
      auto& elementFactory =                                                  \
          ::sophon_stream::framework::SingletonElementFactory::getInstance(); \
      std::cout << elementName << std::endl;                                  \
      elementFactory.addElementMaker(                                         \
          elementName, []() { return std::make_shared<GroupElement>(); });    \
    }                                                                         \
  };                                                                          \
  static Group##ElementClass##Register g##Group##ElementClass##Register;

}  // namespace framework
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_FRAMEWORK_ELEMENT_FACTORY_H_