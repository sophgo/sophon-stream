//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_BYTETRACK_H_
#define SOPHON_STREAM_ELEMENT_BYTETRACK_H_

#include "bytetrack_bytetracker.h"

namespace sophon_stream {
namespace element {
namespace bytetrack {

struct BytetrackContext {
  float trackThresh;
  float highThresh;
  float matchThresh;
  int frameRate;
  int trackBuffer;
};

class Bytetrack : public ::sophon_stream::framework::Element {
 public:
  Bytetrack();
  ~Bytetrack() override;

  Bytetrack(const Bytetrack&) = delete;
  Bytetrack& operator=(const Bytetrack&) = delete;
  Bytetrack(Bytetrack&&) = default;
  Bytetrack& operator=(Bytetrack&&) = default;

  common::ErrorCode initInternal(const std::string& json) override;

  void process(int dataPipeId,
               std::shared_ptr<common::ObjectMetadata>& objectMetadata);

  void uninitInternal() override;

  common::ErrorCode doWork(int dataPipeId) override;

 private:
  std::shared_ptr<BytetrackContext> mContext;  // context对象

  common::ErrorCode initContext(const std::string& json);

  std::map<int, std::shared_ptr<BYTETracker>> mByteTrackerMap;
};

}  // namespace bytetrack
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_BYTETRACK_H_