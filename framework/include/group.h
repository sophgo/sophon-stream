//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_FRAMEWORK_GROUP_H_
#define SOPHON_STREAM_FRAMEWORK_GROUP_H_

#include <chrono>
#include <memory>

#include "element_factory.h"

namespace sophon_stream {
namespace framework {

template <typename T,
          typename std::enable_if<std::is_same_v<
              decltype(T::elementName), const std::string>>::type* = nullptr>
class Group : public ::sophon_stream::framework::Element {
 public:
  Group() {}
  ~Group() override {}
  common::ErrorCode initInternal(const std::string& json) {
    common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
    do {
      // json是否正确
      auto configure = nlohmann::json::parse(json, nullptr, false);
      if (!configure.is_object()) {
        errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
        break;
      }

      elementName = T::elementName;

      auto inner_elements_id_it = configure.find(JSON_INNER_ELEMENTS_ID);
      if (inner_elements_id_it != configure.end()) {
        inner_elements_id = inner_elements_id_it->get<std::vector<int>>();
      }

      auto& elementFactory =
          ::sophon_stream::framework::SingletonElementFactory::getInstance();
      preElement =
          std::dynamic_pointer_cast<T>(elementFactory.make(elementName));
      inferElement =
          std::dynamic_pointer_cast<T>(elementFactory.make(elementName));
      postElement =
          std::dynamic_pointer_cast<T>(elementFactory.make(elementName));

      if (!preElement || !inferElement || !postElement) break;

      initElements(json);

    } while (false);
    return errorCode;
  }
  common::ErrorCode doWork(int dataPipeId) {
    std::this_thread::sleep_for(std::chrono::microseconds(40));
    return common::ErrorCode::SUCCESS;
  }

  bool getGroup() override { return true; }

  void groupInsert(
      std::map<int, std::shared_ptr<framework::Element>>& mapPtr) override {
    auto preElement = this->getPreElement();
    auto inferElement = this->getInferElement();
    auto postElement = this->getPostElement();
    if (preElement && inferElement && postElement) {
      auto preId = preElement->getId();
      auto inferId = inferElement->getId();
      auto postId = postElement->getId();
      mapPtr[preId] = preElement;
      mapPtr[inferId] = inferElement;
      mapPtr[postId] = postElement;
    }
  }

  std::shared_ptr<T> getPreElement() { return preElement; }
  std::shared_ptr<T> getInferElement() { return inferElement; }
  std::shared_ptr<T> getPostElement() { return postElement; }

  inline void setListener(ListenThread* p) override {
    // listenThreadPtr = p;
    preElement->setListener(p);
    inferElement->setListener(p);
    postElement->setListener(p);
  }

  virtual void registListenFunc(ListenThread* listener) {
    preElement->registListenFunc(listener);
    inferElement->registListenFunc(listener);
    postElement->registListenFunc(listener);
  }

  void afterConnect(bool is_dst, bool is_src) {
    auto preElement = getPreElement();
    auto postElement = getPostElement();

    if (!preElement || !postElement) {
      // check
      return;
    }

    if (is_dst) {
      std::vector<int> inputPorts = getInputPorts();
      for (auto inputPort : inputPorts) {
        preElement->addInputPort(inputPort);
      }
      preElement->setInputConnectorMap(getInputConnectorMap());

    } else if (is_src) {
      std::vector<int> outputPorts = getOutputPorts();
      for (auto outputPort : outputPorts) {
        postElement->addOutputPort(outputPort);
      }
      postElement->setOutputConnectorMap(getOutputConnectorMap());
    }
  }

  static constexpr const char* CONFIG_INTERNAL_ELEMENT_NAME_FIELD =
      "element_name";
  static constexpr const char* JSON_INNER_ELEMENTS_ID = "inner_elements_id";

 private:
  std::vector<int> inner_elements_id;

  std::shared_ptr<T> preElement;
  std::shared_ptr<T> inferElement;
  std::shared_ptr<T> postElement;

  std::string elementName;

  common::ErrorCode initElements(const std::string& json) {
    int id = this->getId();
    preElement->setId(inner_elements_id[0]);
    inferElement->setId(inner_elements_id[1]);
    postElement->setId(inner_elements_id[2]);

    std::string side = this->getSide();
    preElement->setSide(side);
    inferElement->setSide(side);
    postElement->setSide(side);

    bool flag = this->getSinkElementFlag();
    preElement->setSinkFlag(false);
    inferElement->setSinkFlag(false);
    postElement->setSinkFlag(flag);

    int dev_id = this->getDeviceId();
    preElement->setDeviceId(dev_id);
    inferElement->setDeviceId(dev_id);
    postElement->setDeviceId(dev_id);

    int threadNum = this->getThreadNumber();
    preElement->setThreadNumber(threadNum);
    inferElement->setThreadNumber(threadNum);
    postElement->setThreadNumber(threadNum);

    preElement->initInternal(json);
    preElement->setStage(true, false, false);
    preElement->initProfiler("fps_" + elementName + "_pre", 100);

    auto context = preElement->getContext();
    auto pre = preElement->getPreProcess();
    auto infer = preElement->getInference();
    auto post = preElement->getPostProcess();

    inferElement->setContext(context);
    inferElement->setPreprocess(pre);
    inferElement->setInference(infer);
    inferElement->setPostprocess(post);
    inferElement->setStage(false, true, false);
    inferElement->initProfiler("fps_" + elementName + "_infer", 100);

    postElement->setContext(context);
    postElement->setPreprocess(pre);
    postElement->setInference(infer);
    postElement->setPostprocess(post);
    postElement->setStage(false, false, true);
    postElement->initProfiler("fps_" + elementName + "_post", 100);

    connect(*preElement, 0, *inferElement, 0);
    connect(*inferElement, 0, *postElement, 0);

    return common::ErrorCode::SUCCESS;
  }
};

}  // namespace framework
}  // namespace sophon_stream

#endif