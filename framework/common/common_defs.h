//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_COMMON_COMMON_DEFS_H_
#define SOPHON_STREAM_COMMON_COMMON_DEFS_H_

#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"

#ifdef chip_1688
#include "bmcv_api_ext.h"
#endif

#define STREAM_LIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 1))
#define STREAM_UNLIKELY(expr) (__builtin_expect(static_cast<bool>(expr), 0))

inline std::string concatArgs() { return ""; }

template <typename T, typename... Args>
inline std::string concatArgs(const T& arg, const Args&... args) {
  std::stringstream ss;
  ss << std::string(arg);
  return ss.str() + concatArgs(args...);
}

#define STREAM_CHECK(cond, ...)                                     \
  if (STREAM_UNLIKELY(!(cond))) {                                   \
    std::string msg = concatArgs(__VA_ARGS__);                      \
    std::string error_msg =                                         \
        "Expected " #cond " to be true, but got false. " + (msg);   \
    std::cerr << "\033[0;31m"                                       \
              << "[STREAM_CHECK_ERROR] "                            \
              << "\033[0m"                                          \
              << "\t" << __FILE__ << ": " << __LINE__ << std::endl; \
    std::cerr << "\033[0;31m"                                       \
              << "[STREAM_CHECK_MESSAGE] "                          \
              << "\033[0m"                                          \
              << "\t" << error_msg << std::endl;                    \
    exit(1);                                                        \
  }

/* for multi version compatible */
#if BMCV_VERSION_MAJOR > 1
typedef bmcv_padding_attr_t bmcv_padding_atrr_t;
/**
 * @brief To solve incompatible issue in a2 sdk
 *
 * @param image input bm_image
 * @return bm_status_t BM_SUCCESS change success, other values: change failed.
 */
static inline bm_status_t bm_image_destroy(bm_image& image) {
  return bm_image_destroy(&image);
}
#endif

#if LIBAVCODEC_VERSION_MAJOR > 58
static int avcodec_decode_video2(AVCodecContext* dec_ctx, AVFrame* frame,
                                 int* got_picture, AVPacket* pkt) {
  int ret;
  *got_picture = 0;
  ret = avcodec_send_packet(dec_ctx, pkt);
  if (ret == AVERROR_EOF) {
    ret = 0;
  } else if (ret < 0) {
    char err[256] = {0};
    av_strerror(ret, err, sizeof(err));
    fprintf(stderr, "Error sending a packet for decoding, %s\n", err);
    return -1;
  }
  while (ret >= 0) {
    ret = avcodec_receive_frame(dec_ctx, frame);
    if (ret == AVERROR(EAGAIN)) {
      ret = 0;
      break;
    } else if (ret == AVERROR_EOF) {
      printf("File end!\n");
      avcodec_flush_buffers(dec_ctx);
      ret = 0;
      break;
    } else if (ret < 0) {
      fprintf(stderr, "Error during decoding\n");
      break;
    }
    *got_picture += 1;
    break;
  }
  if (*got_picture > 1) {
    printf("got picture %d\n", *got_picture);
  }
  return ret;
}
static int avcodec_encode_video2(AVCodecContext* avctx, AVPacket* avpkt,
                                 const AVFrame* frame, int* got_packet_ptr) {
  int ret = avcodec_send_frame(avctx, frame);
  if (ret < 0) {
    return ret;
  }
  ret = avcodec_receive_packet(avctx, avpkt);
  if (ret < 0) {
    *got_packet_ptr = 0;
    return ret;
  } else {
    *got_packet_ptr = 1;
  }
  return 0;
}
#define av_find_input_format(x) \
  const_cast<AVInputFormat*>(av_find_input_format(x))
#define avcodec_find_decoder(x) const_cast<AVCodec*>(avcodec_find_decoder(x))
#define av_guess_format(x1, x2, x3) \
  const_cast<AVOutputFormat*>(av_guess_format(x1, x2, x3))
#define avcodec_find_decoder_by_name(x) \
  const_cast<AVCodec*>(avcodec_find_decoder_by_name(x))
#define avcodec_find_encoder_by_name(x) \
  const_cast<AVCodec*>(avcodec_find_encoder_by_name(x));
#endif

#endif