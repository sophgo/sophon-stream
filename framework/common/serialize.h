//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_SERIALIZE_H_
#define SOPHON_STREAM_ELEMENT_SERIALIZE_H_

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <type_traits>
#include <utility>

#include "common/common_defs.h"
#include "common/logger.h"
#include "detected_object_metadata.h"
#include "face_object_metadata.h"
#include "frame.h"
#include "graphics.h"
#include "object_metadata.h"
#include "posed_object_metadata.h"
#include "recognized_object_metadata.h"
#include "segmented_object_metadata.h"
#include "tracked_object_metadata.h"

#define ENABLE_TIME_LOG 0

namespace sophon_stream {
namespace common {

template <typename T>
struct is_smart_pointer_helper : public std::false_type {};

template <typename T>
struct is_smart_pointer_helper<std::shared_ptr<T> > : public std::true_type {};

template <typename T>
struct is_smart_pointer_helper<std::unique_ptr<T> > : public std::true_type {};

template <typename T>
struct is_smart_pointer_helper<std::weak_ptr<T> > : public std::true_type {};

template <typename T>
struct is_smart_pointer
    : public is_smart_pointer_helper<typename std::remove_cv<T>::type> {};

template <typename T>
void extended_to_json(const char* key, nlohmann::json& j, const T& value) {
  if constexpr (is_smart_pointer<T>::value) {
    j[key] = *value;
  } else
    j[key] = value;
}

#define EXTEND_JSON_TO(v1) \
  extended_to_json(#v1, nlohmann_json_j, nlohmann_json_t.v1);

#define EXTEND_JSON_FROM(v1) \
  extended_from_json(#v1, nlohmann_json_j, nlohmann_json_t.v1);

#define NLOHMANN_JSONIFY_ALL_THINGS(Type, ...)                             \
  inline void to_json(nlohmann::json& nlohmann_json_j,                     \
                      const Type& nlohmann_json_t) {                       \
    NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(EXTEND_JSON_TO, __VA_ARGS__)) \
  }

const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/// Encode a char buffer into a base64 string
/**
 * @param input The input data
 * @param len The length of input in bytes
 * @return A base64 encoded string representing input
 */
std::string base64_encode(unsigned char const* input, size_t len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (len--) {
    char_array_3[i++] = *(input++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++) {
        ret += base64_chars[char_array_4[i]];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) {
      ret += base64_chars[char_array_4[j]];
    }

    while ((i++ < 3)) {
      ret += '=';
    }
  }

  return ret;
}

std::string frame_to_base64(Frame& frame) {
#if ENABLE_TIME_LOG
  timeval time1, time2, time3, time4, time5;
  gettimeofday(&time1, NULL);
#endif
  unsigned char* jpegData = nullptr;
  size_t nBytes = 0;

  bm_image bgr_ = *(frame.mSpData);
  bm_handle_t handle_ = bm_image_get_handle(&bgr_);

  bm_image yuv_;

  bm_image_create(handle_, bgr_.height, bgr_.width, FORMAT_YUV420P,
                  bgr_.data_type, &yuv_);
  bm_image_alloc_dev_mem_heap_mask(yuv_, 2);
#if ENABLE_TIME_LOG
  gettimeofday(&time2, NULL);
#endif
  bmcv_image_storage_convert(handle_, 1, &bgr_, &yuv_);
#if ENABLE_TIME_LOG
  gettimeofday(&time3, NULL);
#endif
  bmcv_image_jpeg_enc(handle_, 1, &yuv_, (void**)&jpegData, &nBytes);
#if ENABLE_TIME_LOG
  gettimeofday(&time4, NULL);
#endif
  bm_image_destroy(yuv_);

  std::string res = base64_encode(jpegData, nBytes);
#if ENABLE_TIME_LOG
  gettimeofday(&time5, NULL);
  double time_delta1 =
      1000 * ((time5.tv_sec - time1.tv_sec) +
              (double)(time5.tv_usec - time1.tv_usec) / 1000000.0);
  double time_delta2 =
      1000 * ((time3.tv_sec - time2.tv_sec) +
              (double)(time3.tv_usec - time2.tv_usec) / 1000000.0);
  double time_delta3 =
      1000 * ((time4.tv_sec - time3.tv_sec) +
              (double)(time4.tv_usec - time3.tv_usec) / 1000000.0);
  double time_delta4 =
      1000 * ((time5.tv_sec - time4.tv_sec) +
              (double)(time5.tv_usec - time4.tv_usec) / 1000000.0);

  IVS_INFO(
      "storage convert time = {0}, jpeg_enc time = {1}, base64_enc time = {2}, "
      "total time = {3}",
      time_delta2, time_delta3, time_delta4, time_delta1);
#endif

  delete jpegData;
  return res;
}

void to_json(nlohmann::json& j, Frame frame) {
  j["mChannelId"] = frame.mChannelId;
  j["mFrameId"] = frame.mFrameId;
  j["mTimestamp"] = frame.mTimestamp;
  j["mEndOfStream"] = frame.mEndOfStream;
  j["mSpData"] = frame_to_base64(frame);
}

NLOHMANN_JSONIFY_ALL_THINGS(TrackedObjectMetadata, mTrackId)

NLOHMANN_JSONIFY_ALL_THINGS(Rectangle<int>, mX, mY, mWidth, mHeight)

NLOHMANN_JSONIFY_ALL_THINGS(DetectedObjectMetadata, mLabelName, mBox, mScores,
                            mClassify)

NLOHMANN_JSONIFY_ALL_THINGS(PosedObjectMetadata, keypoints)

NLOHMANN_JSONIFY_ALL_THINGS(RecognizedObjectMetadata, mLabelName, mScores,
                            mTopKLabels)

NLOHMANN_JSONIFY_ALL_THINGS(SegmentedObjectMetadata, mFrame)

NLOHMANN_JSONIFY_ALL_THINGS(FaceObjectMetadata, top, bottom, left, right,
                            points_x, points_y, score)

void to_json(nlohmann::json& j, std::shared_ptr<common::ObjectMetadata> obj) {
  for (auto detObj : obj->mDetectedObjectMetadatas) {
    j["mDetectedObjectMetadatas"].push_back(*detObj);
  }
  for (auto trackObj : obj->mTrackedObjectMetadatas) {
    j["mTrackedObjectMetadatas"].push_back(*trackObj);
  }
  for (auto poseObj : obj->mPosedObjectMetadatas) {
    j["mPosedObjectMetadatas"].push_back(*poseObj);
  }
  for (auto recogObj : obj->mRecognizedObjectMetadatas) {
    j["mRecognizedObjectMetadatas"].push_back(*recogObj);
  }
  for (auto faceObj : obj->mFaceObjectMetadatas) {
    j["mFaceObjectMetadata"].push_back(*faceObj);
  }
  j["mFrame"] = (*(obj->mFrame));
  for (auto subObj : obj->mSubObjectMetadatas) {
    nlohmann::json subJ;
    to_json(subJ, subObj);
    j["mSubObjectMetadatas"].push_back(subJ);
  }
}

}  // namespace common
}  // namespace sophon_stream

#endif