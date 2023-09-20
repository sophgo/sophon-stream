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

#include "common/logger.h"

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
  Encoder_CC(int dev_id, const std::string& enc_fmt, const std::string& pix_fmt,
             const std::map<std::string, int>& enc_params, int channel_idx);

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
  std::map<std::string, int> enc_params_;
  std::map<std::string, int> params_map_;
  FILE* fp;

  bool is_jpeg_;
  bool is_video_file_;
  bool is_rtsp_;
  bool is_rtmp_;
  bool opened_;

  int channel_idx;

  void* jpeg_addr;
  int jpeg_size;

  cv::Mat mat;
  cv::VideoWriter writer;

  AVPixelFormat pix_fmt_;
  // AVFrame* frame_;
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

  bool pushQueue(std::shared_ptr<void> p);
  std::shared_ptr<void> popQueue();
  int getSize() const;
  void flowControlFunc();

  static constexpr const int queueMaxSize = 10;
  static constexpr const int queueMinSize = 5;
  std::queue<std::shared_ptr<void>> encoderQueue;
  mutable std::mutex mQueueMtx;
  std::thread flow_control;
  bool isRunning = true;
  ::sophon_stream::common::FpsProfiler mFpsProfiler;
};

Encoder::Encoder() : _impl(new Encoder_CC()) {}

Encoder::Encoder(int dev_id, const std::string& enc_fmt,
                 const std::string& pix_fmt,
                 const std::map<std::string, int>& enc_params, int channel_idx)
    : _impl(new Encoder_CC(dev_id, enc_fmt, pix_fmt, enc_params, channel_idx)) {
}

Encoder::~Encoder() { delete _impl; }

bool Encoder::is_opened() { return _impl->is_opened(); }

int Encoder::video_write(bm_image& image) { return _impl->video_write(image); }

void Encoder::set_output_path(const std::string& output_path) {
  return _impl->set_output_path(output_path);
}

void Encoder::release() { return _impl->release(); }
void Encoder::init_writer() { return _impl->init_writer(); }
void Encoder::set_enc_params_width(int width) {
  return _impl->set_enc_params_width(width);
}
void Encoder::set_enc_params_height(int height) {
  return _impl->set_enc_params_height(height);
}

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
  // bitrate用于控制视频的压缩率和质量。较高的比特率会导致更高的数据传输速率和更好的图像质量，但会占用更多的存储空间和带宽。较低的比特率则会减小数据传输速率和存储空间，但会导致图像质量的损失。
  params_map_.insert(std::pair<std::string, int>("bitrate", 2000));
  // 设置适当的gop_size值可以影响视频的压缩效率、编码质量和随机访问性能。较小的gop_size值会增加编码的压缩效率，但也会增加解码的计算复杂度。较大的gop_size值则会提供更好的随机访问性能，但可能会影响压缩效率。
  params_map_.insert(std::pair<std::string, int>("gop", 32));
  // gop_preset参数允许你选择不同的预设类型，以快速设置GOP相关参数，gop_preset=3对应的是veryfast预设类型。
  params_map_.insert(std::pair<std::string, int>("gop_preset", 3));
  params_map_.insert(std::pair<std::string, int>("mb_rc", 0));
  params_map_.insert(std::pair<std::string, int>("qp", -1));
  params_map_.insert(std::pair<std::string, int>("bg", 0));
  params_map_.insert(std::pair<std::string, int>("nr", 0));
  params_map_.insert(std::pair<std::string, int>("weightp", 0));

  // 遍历enc_params_，并将它的键值对插入到params_map_中
  for (auto it = enc_params_.begin(); it != enc_params_.end(); ++it)
    params_map_[it->first] = it->second;
}

Encoder::Encoder_CC::Encoder_CC() {}

Encoder::Encoder_CC::Encoder_CC(int dev_id, const std::string& enc_fmt,
                                const std::string& pix_fmt,
                                const std::map<std::string, int>& enc_params,
                                int channel_idx)
    : index(0),
      is_jpeg_(false),
      is_rtsp_(false),
      is_rtmp_(false),
      is_video_file_(false),
      opened_(false),
      enc_ctx_(nullptr),
      enc_dict_(nullptr),
      enc_fmt_(enc_fmt),
      enc_params_(enc_params),
      pix_fmt_(AV_PIX_FMT_NONE),
      channel_idx(channel_idx) {
  bm_dev_request(&handle_, dev_id);
  enc_params_prase();
  if (pix_fmt == "I420") {
    pix_fmt_ = AV_PIX_FMT_YUV420P;
  } else if (pix_fmt == "NV12") {
    pix_fmt_ = AV_PIX_FMT_NV12;
  } else {
  }
  mFpsProfiler.config("encoder", 100);
  flow_control = std::thread(&Encoder::Encoder_CC::flowControlFunc, this);
}

void Encoder::Encoder_CC::init_writer() {
  if (output_path_.compare(0, 7, "rtmp://") == 0) {
    is_rtmp_ = true;
    std::string enParams =
        "gop=" + std::to_string(params_map_["gop"]) +
        ":gop_preset=" + std::to_string(params_map_["gop_preset"]) +
        ":bitrate=" + std::to_string(params_map_["bitrate"]);
    if (enc_fmt_ == "h264_bm") {
      writer.open(output_path_, cv::VideoWriter::fourcc('H', '2', '6', '4'),
                  params_map_["framerate"],
                  cv::Size(params_map_["width"], params_map_["height"]),
                  enParams, true, bm_get_devid(handle_));

    } else if (enc_fmt_ == "h265_bm") {
      writer.open(output_path_, cv::VideoWriter::fourcc('h', 'v', 'c', '1'),
                  params_map_["framerate"],
                  cv::Size(params_map_["width"], params_map_["height"]),
                  enParams, true, bm_get_devid(handle_));
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
    av_dict_set_int(&enc_dict_, "is_dma_buffer", 1, 0);
    // av_dict_set(&enc_dict_, "rtsp_transport", "tcp", 0);

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
    // timeval tv1, tv2;
    // gettimeofday(&tv1, NULL);
    int ret = bmcv_image_vpp_convert(handle, 1, *image, yuv_image, &crop_rect);
    // gettimeofday(&tv2, NULL);
    // int time_interval1 =
    //       (tv2.tv_sec - tv1.tv_sec) * 1000 * 1000 + (tv2.tv_usec -
    //       tv1.tv_usec);
    // IVS_INFO("Encoder idx is {0}, vpp convert cost is {1}", channel_idx,
    //            time_interval1);
    if (BM_SUCCESS != ret) {
      // gettimeofday(&tv1, NULL);
      ret = bmcv_image_storage_convert(handle, 1, image, yuv_image);
      // gettimeofday(&tv2, NULL);
      // int time_interval2 =
      //     (tv2.tv_sec - tv1.tv_sec) * 1000 * 1000 + (tv2.tv_usec - tv1.tv_usec);
      // IVS_INFO("Encoder idx is {0}, convert cost is {1}", channel_idx,
      //          time_interval2);
      if (BM_SUCCESS != ret) {
        return ret;
      }
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

bool Encoder::Encoder_CC::pushQueue(std::shared_ptr<void> p) {
  if (getSize() >= queueMaxSize) {
    return false;
  }
  std::lock_guard<std::mutex> lock(mQueueMtx);
  encoderQueue.push(p);
  return true;
}

std::shared_ptr<void> Encoder::Encoder_CC::popQueue() {
  std::lock_guard<std::mutex> lock(mQueueMtx);
  std::shared_ptr<void> p = nullptr;
  // printf("encode queue popping! queue size is %d\n", encoderQueue.size());
  if (!encoderQueue.empty()) {
    p = encoderQueue.front();
    encoderQueue.pop();
  }
  return p;
}

int Encoder::Encoder_CC::getSize() const {
  std::lock_guard<std::mutex> lock(mQueueMtx);
  return encoderQueue.size();
}

void Encoder::Encoder_CC::flowControlFunc() {
  while (isRunning && getSize() < queueMinSize) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  while (isRunning) {
    auto p = popQueue();
    if (p == nullptr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      continue;
    }
    int64_t push_start_time =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    if (is_rtsp_ || is_video_file_) {
      std::shared_ptr<AVPacket> pp = std::static_pointer_cast<AVPacket>(p);
      auto ret = av_interleaved_write_frame(enc_format_ctx_, pp.get());
    } else if (is_rtmp_) {
      std::shared_ptr<cv::Mat> pp = std::static_pointer_cast<cv::Mat>(p);
      writer.write(*pp);
    }
    float tmp_fps = mFpsProfiler.getTmpFps();
    int64_t frame_interval =
        1 * 1000 * 1000 / (tmp_fps == 0 ? params_map_["framerate"] : tmp_fps);
    int64_t push_ok = std::chrono::duration_cast<std::chrono::microseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
    int64_t diff = push_ok - push_start_time;
    // printf("encode diff time is %lld\n", diff);
    if (diff < frame_interval && diff > 0) av_usleep(frame_interval - diff);
  }
  return;
}

int Encoder::Encoder_CC::video_write(bm_image& image) {
  if (is_rtsp_ || is_video_file_) {
    // auto _start_time = std::chrono::high_resolution_clock::now();

    int ret = 0;
    int got_output = 0;
    // int64_t start_time = 0;
    std::shared_ptr<AVFrame> frame_ = nullptr;
    frame_.reset(av_frame_alloc(), [](AVFrame* p) {
      if (p != nullptr) {
        av_frame_unref(p);
        av_frame_free(&p);
      }
    });
    std::shared_ptr<AVPacket> test_enc_pkt(new AVPacket);

    ret = bm_image_to_avframe(handle_, &image, frame_.get());
    if (ret < 0) return -1;
    test_enc_pkt->data = NULL;
    test_enc_pkt->size = 0;
    av_init_packet(test_enc_pkt.get());

    ret = avcodec_encode_video2(enc_ctx_, test_enc_pkt.get(), frame_.get(),
                                &got_output);
    if (ret < 0) return ret;
    if (got_output == 0) {
      return -1;
    }
    av_packet_rescale_ts(test_enc_pkt.get(), enc_ctx_->time_base,
                         out_stream_->time_base);
    mFpsProfiler.add(1);
    pushQueue(std::static_pointer_cast<void>(test_enc_pkt));

    // auto _finish_time = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> dur = _finish_time - _start_time;
    // IVS_DEBUG("Time Cost on video_write is {0}, Encoder ID is {1}", dur.count(),
    //           channel_idx);
    return ret;
  }
  if (is_rtmp_) {
    cv::Mat write_mat;
    cv::bmcv::toMAT(&image, write_mat, true);
    mFpsProfiler.add(1);
    pushQueue(
        std::static_pointer_cast<void>(std::make_shared<cv::Mat>(write_mat)));
    return 0;
  }
}

int Encoder::Encoder_CC::flush_encoder() {
  int ret;
  int got_frame = 0;
  if (!(this->enc_ctx_->codec->capabilities & AV_CODEC_CAP_DELAY)) return 0;
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
  isRunning = false;
  flow_control.join();
  if (enc_ctx_) {
    flush_encoder();
    av_write_trailer(enc_format_ctx_);
  }

  if (enc_dict_) av_dict_free(&enc_dict_);
  if (enc_ctx_) avcodec_free_context(&enc_ctx_);
  opened_ = false;
  return;
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
