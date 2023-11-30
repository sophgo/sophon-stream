//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_BLANK_H_
#define SOPHON_STREAM_ELEMENT_BLANK_H_

#include "common/object_metadata.h"
#include "element.h"

namespace sophon_stream {
namespace element {
namespace blank {

class Blank : public ::sophon_stream::framework::Element {
 public:
  Blank();
  ~Blank() override;

  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  std::string postNameSetIdx = "/blank/setIdx";
  void listenerSetIdx(const httplib::Request& request,
                      httplib::Response& response);
  void registListenFunc(sophon_stream::framework::ListenThread* listener) override;

 private:
  int printIdx;
};

}  // namespace blank
}  // namespace element
}  // namespace sophon_stream

#endif