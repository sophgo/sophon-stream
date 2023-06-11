//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "encoder.h"

#include <sys/time.h>

#include <iostream>

namespace sophon_stream {
namespace element {
namespace encode {

typedef struct {
  bm_image* bmImg;
  uint8_t* buf0;
  uint8_t* buf1;
  uint8_t* buf2;
} transcode_t;

void bmBufferDeviceMemFree(void* opaque, uint8_t* data) {
  if (opaque == NULL) {
  }
  transcode_t* testTranscoed = (transcode_t*)opaque;
  av_freep(&testTranscoed->buf0);
  testTranscoed->buf0 = NULL;

  int ret = 0;
  ret = bm_image_destroy(*(testTranscoed->bmImg));
  if (testTranscoed->bmImg) {
    free(testTranscoed->bmImg);
    testTranscoed->bmImg = NULL;
  }
  if (ret != 0) free(testTranscoed);
  testTranscoed = NULL;
  return;
}
void bmBufferDeviceMemFreeEmpty(void* opaque, uint8_t* data) { return; }

class Encoder::Encoder_CC {
 public:
  Encoder_CC();
  Encoder_CC(bm_handle_t& handle, const std::string& enc_fmt,
             const std::string& pix_fmt, const std::string& enc_params);

  ~Encoder_CC();

  void set_output_path(const std::string& output_path);
  void init_writer();
  bool is_opened();
  int video_write(bm_image& image);
  void release();
  void set_enc_params_width(int width);
  void set_enc_params_height(int height);

 private:
  int index;
  int tpu_id_;
  bm_handle_t handle_;
  std::string output_path_;
  std::string enc_fmt_;
  std::string enc_params_;
  std::map<std::string, int> params_map_;
  FILE* fp;

  bool is_jpeg_;
  bool is_video_file_;
  bool is_rtsp_;
  bool is_rtmp_;
  bool opened_;

  void* jpeg_addr;
  int jpeg_size;

  cv::Mat mat;
  cv::VideoWriter writer;

  AVPixelFormat pix_fmt_;
  AVFrame* frame_;
  AVCodec* encoder_;
  AVDictionary* enc_dict_;
  AVIOContext* avio_ctx_;
  AVFormatContext* enc_format_ctx_;
  AVOutputFormat* enc_output_fmt_;
  AVCodecContext* enc_ctx_;
  AVStream* out_stream_;
  AVPacket* pkt_;

  void enc_params_prase();
  int map_bmformat_to_avformat(int bmformat);
  int bm_image_to_avframe(bm_handle_t& handle, bm_image* image, AVFrame* frame);
  int flush_encoder();
};

Encoder::Encoder() : _impl(new Encoder_CC()) {}

Encoder::Encoder(bm_handle_t& handle, const std::string& enc_fmt,
                 const std::string& pix_fmt, const std::string& enc_params)
    : _impl(new Encoder_CC(handle, enc_fmt, pix_fmt, enc_params)) {}

Encoder::~Encoder() { delete _impl; }

bool Encoder::is_opened() { return _impl->is_opened(); }

int Encoder::video_write(bm_image& image) { return _impl->video_write(image); }
void Encoder::set_output_path(const std::string& output_path) {
  return _impl->set_output_path(output_path);
}

void Encoder::release() { return _impl->release(); }
void Encoder::init_writer() { return _impl->init_writer(); }
void Encoder::set_enc_params_width(int width) { return _impl->set_enc_params_width(width); }
void Encoder::set_enc_params_height(int height) { return _impl->set_enc_params_height(height); }

int Encoder::Encoder_CC::map_bmformat_to_avformat(int bmformat) {
  int format = 0;
  if (is_jpeg_) {
    switch (bmformat) {
      case FORMAT_YUV420P:
        format = AV_PIX_FMT_YUVJ420P;
        break;
      case FORMAT_YUV422P:
        format = AV_PIX_FMT_YUVJ422P;
        break;
      case FORMAT_YUV444P:
        format = AV_PIX_FMT_YUVJ444P;
        break;
      default:
        format = -1;
    }
  } else {
    switch (bmformat) {
      case FORMAT_YUV420P:
        format = AV_PIX_FMT_YUV420P;
        break;
      case FORMAT_NV12:
        format = AV_PIX_FMT_NV12;
        break;
      default:
        format = -1;
    }
  }
  return format;
}

void Encoder::Encoder_CC::enc_params_prase() {
  // params_map_.insert(std::pair<std::string, int>("width", 1920));
  // params_map_.insert(std::pair<std::string, int>("height", 1080));
  params_map_.insert(std::pair<std::string, int>("framerate", 25));
  params_map_.insert(std::pair<std::string, int>("bitrate", 2000));
  params_map_.insert(std::pair<std::string, int>("gop", 32));
  params_map_.insert(std::pair<std::string, int>("gop_preset", 3));
  params_map_.insert(std::pair<std::string, int>("mb_rc", 0));
  params_map_.insert(std::pair<std::string, int>("qp", -1));
  params_map_.insert(std::pair<std::string, int>("bg", 0));
  params_map_.insert(std::pair<std::string, int>("nr", 0));
  params_map_.insert(std::pair<std::string, int>("weightp", 0));

  std::string s1;
  s1.append(1, ':');
  std::regex reg1(s1);

  std::string s2;
  s2.append(1, '=');
  std::regex reg2(s2);

  std::vector<std::string> elems(
      std::sregex_token_iterator(enc_params_.begin(), enc_params_.end(), reg1,
                                 -1),
      std::sregex_token_iterator());
  for (auto param : elems) {
    std::vector<std::string> key_value_(
        std::sregex_token_iterator(param.begin(), param.end(), reg2, -1),
        std::sregex_token_iterator());

    std::string temp_key = key_value_[0];
    std::string temp_value = key_value_[1];

    params_map_[temp_key] = std::stoi(temp_value);
  }
}

Encoder::Encoder_CC::Encoder_CC() {}

Encoder::Encoder_CC::Encoder_CC(bm_handle_t& handle, const std::string& enc_fmt,
                                const std::string& pix_fmt,
                                const std::string& enc_params)
    : index(0),
      handle_(handle),
      is_jpeg_(false),
      is_rtsp_(false),
      is_rtmp_(false),
      is_video_file_(false),
      opened_(false),
      enc_ctx_(nullptr),
      enc_dict_(nullptr),
      enc_fmt_(enc_fmt),
      enc_params_(enc_params),
      pix_fmt_(AV_PIX_FMT_NONE) {
  enc_params_prase();
  if (pix_fmt == "I420") {
    pix_fmt_ = AV_PIX_FMT_YUV420P;
  } else if (pix_fmt == "NV12") {
    pix_fmt_ = AV_PIX_FMT_NV12;
  } else {
  }
}

void Encoder::Encoder_CC::init_writer() {
  if (output_path_.compare(0, 7, "rtmp://") == 0) {
    is_rtmp_ = true;
    if (enc_fmt_ == "h264_bm") {
      writer.open(output_path_, cv::VideoWriter::fourcc('H', '2', '6', '4'),
                  params_map_["framerate"],
                  cv::Size(params_map_["width"], params_map_["height"]),
                  enc_params_, true, bm_get_devid(handle_));

    } else if (enc_fmt_ == "h265_bm") {
      writer.open(output_path_, cv::VideoWriter::fourcc('h', 'v', 'c', '1'),
                  params_map_["framerate"],
                  cv::Size(params_map_["width"], params_map_["height"]),
                  enc_params_, true, bm_get_devid(handle_));
    } else {
    }
    opened_ = true;
  } else {
    if (output_path_.compare(0, 7, "rtsp://") == 0) {
      is_rtsp_ = true;
      avformat_alloc_output_context2(&enc_format_ctx_, NULL, "rtsp",
                                     output_path_.c_str());
      if (!enc_format_ctx_) {
      }
    } else {
      is_video_file_ = true;
      avformat_alloc_output_context2(&enc_format_ctx_, NULL, NULL,
                                     output_path_.c_str());
      enc_output_fmt_ = av_guess_format(NULL, output_path_.c_str(), NULL);
      if (enc_output_fmt_->video_codec == AV_CODEC_ID_NONE) {
      }
      enc_format_ctx_->oformat = enc_output_fmt_;
    }

    encoder_ = avcodec_find_encoder_by_name(enc_fmt_.c_str());
    if (!encoder_) {
    }
    enc_ctx_ = avcodec_alloc_context3(encoder_);
    if (!encoder_) {
    }

    enc_ctx_->codec_id = encoder_->id;
    enc_ctx_->pix_fmt = pix_fmt_;

    enc_ctx_->width = params_map_["width"];
    enc_ctx_->height = params_map_["height"];
    enc_ctx_->gop_size = params_map_["gop"];
    enc_ctx_->time_base = (AVRational){1, params_map_["framerate"]};
    enc_ctx_->framerate = (AVRational){params_map_["framerate"], 1};

    av_dict_set_int(&enc_dict_, "sophon_idx", bm_get_devid(handle_), 0);
    av_dict_set_int(&enc_dict_, "gop_preset", params_map_["gop_preset"], 0);
    // av_dict_set_int(&enc_dict_, "mb_rc",      params_map_["mb_rc"],      0);
    // 0); av_dict_set_int(&enc_dict_, "bg",         params_map_["bg"], 0);
    // av_dict_set_int(&enc_dict_, "nr",         params_map_["nr"],         0);
    // av_dict_set_int(&enc_dict_, "weightp",    params_map_["weightp"],    0);
    av_dict_set_int(&enc_dict_, "is_dma_buffer", 1, 0);

    if (-1 == params_map_["qp"]) {
      enc_ctx_->bit_rate_tolerance = params_map_["bitrate"] * 1000;
      enc_ctx_->bit_rate = (int64_t)params_map_["bitrate"] * 1000;
    } else {
      av_dict_set_int(&enc_dict_, "qp", params_map_["qp"], 0);
    }

    out_stream_ = avformat_new_stream(enc_format_ctx_, encoder_);

    out_stream_->time_base = enc_ctx_->time_base;
    out_stream_->avg_frame_rate = enc_ctx_->framerate;
    out_stream_->r_frame_rate = out_stream_->avg_frame_rate;

    int ret = avcodec_open2(enc_ctx_, encoder_, &enc_dict_);
    if (ret < 0) {
    }
    ret = avcodec_parameters_from_context(out_stream_->codecpar, enc_ctx_);
    if (ret < 0) {
    }
    if (is_video_file_) {
      if (!(enc_format_ctx_->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open2(&enc_format_ctx_->pb, output_path_.c_str(),
                         AVIO_FLAG_WRITE, NULL, NULL);
        if (ret < 0) {
        }
      }
    }
    ret = avformat_write_header(enc_format_ctx_, NULL);
    if (ret < 0) {
    }
    opened_ = true;
  }
}

Encoder::Encoder_CC::~Encoder_CC() {
  // SPDLOG_INFO("release encoder");
  // release();
}

int Encoder::Encoder_CC::bm_image_to_avframe(bm_handle_t& handle,
                                             bm_image* image, AVFrame* frame) {
  if (!bm_image_is_attached(*image)) return -1;
  int plane = 0;
  bm_image* yuv_image = (bm_image*)malloc(sizeof(bm_image));
  bm_image_format_info info;
  int encode_stride = ((image->width + 31) >> 5) << 5;

  if (is_rtsp_ || is_video_file_) {
    if (pix_fmt_ == AV_PIX_FMT_YUV420P) {
      plane = 3;
      int stride_bmi[3] = {encode_stride, encode_stride / 2, encode_stride / 2};
      bm_image_create(handle, image->height, image->width, FORMAT_YUV420P,
                      DATA_TYPE_EXT_1N_BYTE, yuv_image, stride_bmi);
    }
    if (pix_fmt_ == AV_PIX_FMT_NV12) {
      plane = 2;
      int stride_bmi[2] = {encode_stride, encode_stride};
      bm_image_create(handle, image->height, image->width, FORMAT_NV12,
                      DATA_TYPE_EXT_1N_BYTE, yuv_image, stride_bmi);
    }

    bm_image_alloc_dev_mem_heap_mask(*yuv_image, 4);
    bmcv_rect_t crop_rect = {0, 0, image->width, image->height};
    int ret = bmcv_image_vpp_convert(handle, 1, *image, yuv_image, &crop_rect);
    if (BM_SUCCESS != ret) {
      ret = bmcv_image_storage_convert(handle, 1, image, yuv_image);
      if (BM_SUCCESS != ret) {
        return ret;
      }
    }
  }

  if (is_jpeg_) {
    plane = 3;
    if (image->image_format == FORMAT_NV12) {
      int stride_bmi[3] = {encode_stride, encode_stride / 2, encode_stride / 2};
      bm_image_create(handle, image->height, image->width, FORMAT_YUV420P,
                      DATA_TYPE_EXT_1N_BYTE, yuv_image, stride_bmi);
      int ret = bmcv_image_storage_convert(handle, 1, image, yuv_image);
      if (BM_SUCCESS != ret) {
        return ret;
      }
    }

    else if (image->image_format == FORMAT_RGB_PLANAR ||
             image->image_format == FORMAT_BGR_PLANAR ||
             image->image_format == FORMAT_RGB_PACKED ||
             image->image_format == FORMAT_BGR_PACKED) {
      int stride_bmi[3] = {encode_stride, encode_stride / 2, encode_stride / 2};
      bm_image_create(handle, image->height, image->width, FORMAT_YUV420P,
                      DATA_TYPE_EXT_1N_BYTE, yuv_image, stride_bmi);
      int ret = bmcv_image_vpp_csc_matrix_convert(handle, 1, *image, yuv_image,
                                                  CSC_RGB2YPbPr_BT601);
      if (BM_SUCCESS != ret) {
        ret = bmcv_image_storage_convert(handle, 1, image, yuv_image);
        if (BM_SUCCESS != ret) {
          return ret;
        }
      }
    } else {
      yuv_image = image;
    }
  }

  transcode_t* ImgOut = NULL;
  ImgOut = (transcode_t*)malloc(sizeof(transcode_t));
  ImgOut->bmImg = yuv_image;

  if (ImgOut->bmImg->width > 0 && ImgOut->bmImg->height > 0 &&
      ImgOut->bmImg->height * ImgOut->bmImg->width <= 8192 * 4096) {
    ImgOut->buf0 = (uint8_t*)av_malloc(ImgOut->bmImg->height *
                                       ImgOut->bmImg->width * 3 / 2);
    ImgOut->buf1 = ImgOut->buf0 +
                   (unsigned int)(ImgOut->bmImg->height * ImgOut->bmImg->width);
    if (plane == 3) {
      ImgOut->buf2 =
          ImgOut->buf0 +
          (unsigned int)(ImgOut->bmImg->height * ImgOut->bmImg->width * 5 / 4);
    }
  }

  frame->buf[0] = av_buffer_create(
      ImgOut->buf0, ImgOut->bmImg->width * ImgOut->bmImg->height,
      bmBufferDeviceMemFree, ImgOut, AV_BUFFER_FLAG_READONLY);
  frame->buf[1] = av_buffer_create(
      ImgOut->buf1, ImgOut->bmImg->width * ImgOut->bmImg->height / 2 / 2,
      bmBufferDeviceMemFreeEmpty, NULL, AV_BUFFER_FLAG_READONLY);
  frame->data[0] = ImgOut->buf0;
  frame->data[1] = ImgOut->buf0;

  if (plane == 3) {
    frame->buf[2] = av_buffer_create(
        ImgOut->buf2, ImgOut->bmImg->width * ImgOut->bmImg->height / 2 / 2,
        bmBufferDeviceMemFreeEmpty, NULL, AV_BUFFER_FLAG_READONLY);
    frame->data[2] = ImgOut->buf0;
  }

  if (plane == 3 && !frame->buf[2]) {
    av_buffer_unref(&frame->buf[0]);
    av_buffer_unref(&frame->buf[1]);
    av_buffer_unref(&frame->buf[2]);
    free(ImgOut);
    free(image);
    return -1;
  } else if (plane == 2 && !frame->buf[1]) {
    av_buffer_unref(&frame->buf[0]);
    av_buffer_unref(&frame->buf[1]);
    free(ImgOut);
    free(image);
    return -1;
  }

  frame->format =
      (AVPixelFormat)map_bmformat_to_avformat(yuv_image->image_format);
  frame->height = image->height;
  frame->width = image->width;

  bm_device_mem_t mems[plane];
  bm_image_get_device_mem(*yuv_image, mems);
  bm_image_get_format_info(yuv_image, &info);

  for (int idx = 0; idx < plane; idx++) {
    frame->data[4 + idx] = (uint8_t*)mems[idx].u.device.device_addr;
    frame->linesize[idx] = info.stride[idx];
    frame->linesize[4 + idx] = info.stride[idx];
  }

  return 0;
}

bool Encoder::Encoder_CC::is_opened() { return opened_; }

int Encoder::Encoder_CC::video_write(bm_image& image) {
  int64_t push_start_time =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  if (is_rtsp_ || is_video_file_) {
    int ret = 0;
    int got_output = 0;
    int64_t start_time = 0;
    frame_ = av_frame_alloc();
    ret = bm_image_to_avframe(handle_, &image, frame_);
    if (ret < 0) return -1;
    AVPacket test_enc_pkt;
    test_enc_pkt.data = NULL;
    test_enc_pkt.size = 0;
    av_init_packet(&test_enc_pkt);
    ret = avcodec_encode_video2(enc_ctx_, &test_enc_pkt, frame_, &got_output);

    if (ret < 0) return ret;
    if (got_output == 0) {
      return -1;
    }
    av_packet_rescale_ts(&test_enc_pkt, enc_ctx_->time_base,
                         out_stream_->time_base);
    ret = av_interleaved_write_frame(enc_format_ctx_, &test_enc_pkt);
    av_frame_unref(frame_);
    av_frame_free(&frame_);
    if (is_rtsp_) {
      int64_t frame_interval = 1 * 1000 * 1000 / params_map_["framerate"];
      int64_t push_ok = std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
      int64_t diff = push_ok - push_start_time;
      if (diff < frame_interval && diff > 0) av_usleep(frame_interval - diff);
    }
    return ret;
  }
  if (is_rtmp_) {
    cv::Mat write_mat;
    cv::bmcv::toMAT(&image, write_mat, true);
    writer.write(write_mat);
    int64_t frame_interval = 1 * 1000 * 1000 / params_map_["framerate"];
    int64_t push_ok = std::chrono::duration_cast<std::chrono::microseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
    int64_t diff = push_ok - push_start_time;
    if (diff < frame_interval && diff > 0) av_usleep(frame_interval - diff);
    return 0;
  }
}

int Encoder::Encoder_CC::flush_encoder() {
  int ret;
  int got_frame = 0;
  if (!(enc_ctx_->codec->capabilities & AV_CODEC_CAP_DELAY)) return 0;
  while (1) {
    av_log(NULL, AV_LOG_INFO, "Flushing video encoder\n");
    AVPacket temp_enc_pkt;
    temp_enc_pkt.data = NULL;
    temp_enc_pkt.size = 0;
    av_init_packet(&temp_enc_pkt);
    ret =
        avcodec_encode_video2(this->enc_ctx_, &temp_enc_pkt, NULL, &got_frame);
    if (ret < 0) return ret;

    if (!got_frame) break;

    av_packet_rescale_ts(&temp_enc_pkt, this->enc_ctx_->time_base,
                         this->out_stream_->time_base);
    /* mux encoded frame */
    av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
    ret = av_interleaved_write_frame(this->enc_format_ctx_, &temp_enc_pkt);
    if (ret < 0) break;
  }
  return ret;
}

void Encoder::Encoder_CC::release() {
  flush_encoder();
  av_write_trailer(enc_format_ctx_);
  // if(pkt_)
  //     av_packet_free(&pkt_);
  if (frame_) av_frame_free(&frame_);
  if (enc_dict_) av_dict_free(&enc_dict_);
  if (enc_ctx_) avcodec_free_context(&enc_ctx_);
  opened_ = false;
}

void Encoder::Encoder_CC::set_output_path(const std::string& output_path) {
  output_path_ = output_path;
}

void Encoder::Encoder_CC::set_enc_params_width(int width) {
  params_map_["width"] = width;
}

void Encoder::Encoder_CC::set_enc_params_height(int height) {
  params_map_["height"] = height;
}

}  // namespace encode
}  // namespace element
}  // namespace sophon_stream
