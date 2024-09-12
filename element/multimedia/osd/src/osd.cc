//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "osd.h"

#include <stdlib.h>

#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

#include "common/common_defs.h"
#include "common/logger.h"
#include "draw_utils.h"
#include "element_factory.h"

namespace sophon_stream {
namespace element {
namespace osd {
Osd::Osd() {}

Osd::~Osd() {
  for (int i = 0; i < overlay_image_.size(); i++)
    bm_image_destroy(overlay_image_[i]);
}

common::ErrorCode Osd::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    // json是否正确
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    mFpsProfiler.config("fps_osd", 100);
    std::string osd_type =
        configure.find(CONFIG_INTERNAL_OSD_TYPE_FIELD)->get<std::string>();
    mOsdType = OsdType::TRACK;
    if (osd_type == "DET") mOsdType = OsdType::DET;
    if (osd_type == "TRACK") mOsdType = OsdType::TRACK;
    if (osd_type == "POSE") mOsdType = OsdType::POSE;
    if (osd_type == "AREA") mOsdType = OsdType::AREA;
    if (osd_type == "ALGORITHM") mOsdType = OsdType::ALGORITHM;
    if (osd_type == "TEXT") mOsdType = OsdType::TEXT;
    if (mOsdType == OsdType::TEXT) {
      auto leftIt = configure.find(CONFIG_INTERNAL_LEFT_FIELD);

      STREAM_CHECK((leftIt != configure.end() && leftIt->is_array()),
                   "left must be array, please check your osd element "
                   "configuration file");
      for (auto& left_obj : *leftIt) {
        int left = left_obj.get<int>();
        lefts.push_back(left);
      }
      auto topIt = configure.find(CONFIG_INTERNAL_TOP_FIELD);

      STREAM_CHECK((topIt != configure.end() && topIt->is_array()),
                   "top must be array, please check your osd element "
                   "configuration file");
      for (auto& top_obj : *leftIt) {
        int top = top_obj.get<int>();
        tops.push_back(top);
      }
      auto textIt = configure.find(CONFIG_INTERNAL_TEXT_FIELD);

      STREAM_CHECK((textIt != configure.end() && textIt->is_array()),
                   "text must be array, please check your osd element "
                   "configuration file");
      for (auto& text_obj : *textIt) {
        std::string text = text_obj.get<std::string>();
        texts.push_back(text);
      }
      font_library = configure.find(CONFIG_INTERNAL_FONT_LIBRARY_FIELD)
                         ->get<std::string>();
      r = configure.find(CONFIG_INTERNAL_R_FIELD)->get<int>();
      g = configure.find(CONFIG_INTERNAL_G_FIELD)->get<int>();
      b = configure.find(CONFIG_INTERNAL_B_FIELD)->get<int>();

      uni_text::UniText uniText(font_library.c_str(), 30);
      bm_handle_t h;
      bm_dev_request(&h, 0);
      for (int i = 0; i < texts.size(); i++) {
        bm_image overlay_image;
        uniText.genBitMap(h, texts[i], overlay_image, r, g, b);
        overlay_image_.push_back(overlay_image);
      }
      if (bmcv_image_overlay == nullptr) {
        IVS_ERROR(
            "bmcv_image_overlay not support,please check your config file or "
            "update SDK version");
        abort();
      }
    }
    if (mOsdType == OsdType::DET) {
      std::string class_names_file =
          configure.find(CONFIG_INTERNAL_CLASS_NAMES_FIELD)->get<std::string>();
      std::ifstream istream;
      istream.open(class_names_file);
      assert(istream.is_open());
      std::string line;
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        mClassNames.push_back(line);
      }
      istream.close();
    }
    if (mOsdType == OsdType::ALGORITHM &&
        configure.end() != configure.find(CONFIG_INTERNAL_CLASS_NAMES_FIELD)) {
      std::string class_names_file =
          configure.find(CONFIG_INTERNAL_CLASS_NAMES_FIELD)->get<std::string>();
      std::ifstream istream;
      istream.open(class_names_file);
      assert(istream.is_open());
      std::string line;
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        mClassNames.push_back(line);
      }
      istream.close();
    }
    if (mOsdType == OsdType::ALGORITHM &&
        configure.end() !=
            configure.find(CONFIG_INTERNAL_RECOGNIZE_NAMES_FIELD)) {
      std::string recognize_names_file =
          configure.find(CONFIG_INTERNAL_RECOGNIZE_NAMES_FIELD)
              ->get<std::string>();
      std::ifstream istream;
      istream.open(recognize_names_file);
      assert(istream.is_open());
      std::string line;
      while (std::getline(istream, line)) {
        line = line.substr(0, line.length());
        mRecognizeNames.push_back(line);
      }
      istream.close();
    }
    mDrawUtils = DrawUtils::OPENCV;
    auto drawUtilsIt = configure.find(CONFIG_INTERNAL_DRAW_UTILS_FIELD);
    if (configure.end() != drawUtilsIt) {
      auto drawUtils = drawUtilsIt->get<std::string>();
      if (drawUtils == "OPENCV") mDrawUtils = DrawUtils::OPENCV;
      if (drawUtils == "BMCV") mDrawUtils = DrawUtils::BMCV;
      IVS_DEBUG("drawUtils is {0}", drawUtils);
    } else {
      IVS_ERROR(
          "Can not find {0} in osd json configure, "
          "json:{1}, set default OPENCV",
          CONFIG_INTERNAL_DRAW_UTILS_FIELD, json);
    }

    mDrawInterval = false;
    auto drawIntervalIt = configure.find(CONFIG_INTERNAL_DRAW_INTERVAL_FIELD);
    if (configure.end() != drawIntervalIt) {
      mDrawInterval = drawIntervalIt->get<bool>();
      IVS_DEBUG("mDrawInterval is {0}", mDrawInterval);
    } else {
      IVS_ERROR(
          "Can not find {0} in osd json configure, "
          "json:{1}, set default false",
          CONFIG_INTERNAL_DRAW_INTERVAL_FIELD, json);
    }

    mPutText = false;
    auto putTestIt = configure.find(CONFIG_INTERNAL_PUT_TEXT_FIELD);
    if (configure.end() != putTestIt) {
      mPutText = putTestIt->get<bool>();
      IVS_DEBUG("mPutTest is {0}", mPutText);
    } else {
      IVS_ERROR(
          "Can not find {0} in osd json configure, "
          "json:{1}, set default true",
          CONFIG_INTERNAL_PUT_TEXT_FIELD, json);
    }
    auto heatmaplossIt = configure.find(CONFIG_INTERNAL_HEATMAP_LOSS_FIELD);
    if (configure.end() != heatmaplossIt) {
      auto heatmaploss = heatmaplossIt->get<std::string>();
      heatmap_loss = heatmaploss;
    }
    if (mOsdType == OsdType::ALGORITHM) {
      std::string draw_func_name =
          configure.find(CONFIG_INTERNAL_DRAW_FUNC_NAME_FIELD)
              ->get<std::string>();
      if (draw_func_name == "draw_bytetrack_results")
        draw_func = std::bind(draw_bytetrack_results, std::placeholders::_1,
                              std::placeholders::_2);
      else if (draw_func_name == "draw_license_plate_recognition_results")
        draw_func_opencv =
            std::bind(draw_license_plate_recognition_results_opencv,
                      std::placeholders::_1, std::placeholders::_2);
      else if (draw_func_name == "draw_retinaface_results")
        draw_func = std::bind(draw_retinaface_results, std::placeholders::_1,
                              std::placeholders::_2);
      else if (draw_func_name ==
               "draw_retinaface_distributor_resnet_faiss_converger_results")
        draw_func = std::bind(
            draw_retinaface_distributor_resnet_faiss_converger_results,
            std::placeholders::_1, std::placeholders::_2);
      else if (draw_func_name == "draw_yolov5_results")
        draw_func = std::bind(draw_yolov5_results, std::placeholders::_1,
                              std::placeholders::_2, mClassNames);
      else if (draw_func_name ==
               "draw_yolov5_bytetrack_distributor_resnet_converger_results")
        draw_func = std::bind(
            draw_yolov5_bytetrack_distributor_resnet_converger_results,
            std::placeholders::_1, std::placeholders::_2, mClassNames,
            mRecognizeNames);
      else if (draw_func_name == "draw_yolox_results")
        draw_func = std::bind(draw_yolox_results, std::placeholders::_1,
                              std::placeholders::_2, mClassNames);
      else if (draw_func_name == "draw_yolov7_results")
        draw_func = std::bind(draw_yolov5_results, std::placeholders::_1,
                              std::placeholders::_2, mClassNames);

      else if (draw_func_name == "draw_yolov5_fastpose_posec3d_results")
        draw_func = std::bind(draw_yolov5_fastpose_posec3d_results,
                              std::placeholders::_1, std::placeholders::_2,
                              heatmap_loss);
      else if (draw_func_name == "default")
        draw_func = std::function<void(
            std::shared_ptr<sophon_stream::common::ObjectMetadata>, bm_image&)>(
            draw_default);
      else if (draw_func_name == "draw_ppocr_results")
        draw_func_opencv = std::bind(draw_ppocr_results, std::placeholders::_1,
                                     std::placeholders::_2);
      else if (draw_func_name == "draw_yolov8_det_pose")
        draw_func_opencv = std::bind(
            draw_yolov8_det_pose, std::placeholders::_1, std::placeholders::_2);
      else
        IVS_ERROR("No such function! Please check your 'draw_func_name'.");
    }

  } while (false);
  return errorCode;
}

common::ErrorCode Osd::doWork(int dataPipeId) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;

  common::ObjectMetadatas objectMetadatas;
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  std::shared_ptr<void> data;
  while (getThreadStatus() == ThreadStatus::RUN) {
    data = popInputData(inputPort, dataPipeId);
    if (!data) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    break;
  }

  if (!data) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);
  if (!(objectMetadata->mFrame->mEndOfStream) &&
      std::find(objectMetadata->mSkipElements.begin(),
                objectMetadata->mSkipElements.end(),
                getId()) == objectMetadata->mSkipElements.end()) {
    draw(objectMetadata);
    mFpsProfiler.add(1);
  }

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  errorCode = pushOutputData(outputPort, outDataPipeId, objectMetadata);
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }

  return common::ErrorCode::SUCCESS;
}
void Osd::draw(std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  std::shared_ptr<bm_image> imageStorage;
  imageStorage.reset(new bm_image, [&](bm_image* img) {
    bm_image_destroy(*img);
    delete img;
    img = nullptr;
  });
  bm_image image = *(objectMetadata->mFrame->mSpData);
  if (mDrawUtils == DrawUtils::OPENCV) {
    cv::Mat frame_to_draw;
    cv::bmcv::toMAT(&image, frame_to_draw);
    switch (mOsdType) {
      case OsdType::DET:
        draw_opencv_det_result(objectMetadata, mClassNames, frame_to_draw,
                               mPutText, mDrawInterval);
        break;

      case OsdType::TRACK:
        draw_opencv_track_result(objectMetadata, mClassNames, frame_to_draw,
                                 mPutText, mDrawInterval);
        break;

      case OsdType::POSE:
        draw_opencv_pose_result(objectMetadata->mFrame->mHandle, objectMetadata,
                                frame_to_draw, mDrawInterval);
        break;

      case OsdType::AREA:
        draw_opencv_areas(objectMetadata, frame_to_draw);
        break;
      case OsdType::ALGORITHM:
        draw_func_opencv(objectMetadata, frame_to_draw);
        break;
      default:
        IVS_WARN("osd_type not support");
    }
    cv::bmcv::toBMI(frame_to_draw, &(*imageStorage));
    if ((*imageStorage).image_format != FORMAT_YUV420P) {
      bm_image frame;
      bm_image_create(objectMetadata->mFrame->mHandle, (*imageStorage).height,
                      (*imageStorage).width, FORMAT_YUV420P,
                      (*imageStorage).data_type, &frame);
      bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1,
                                 &(*imageStorage), &frame);
      bm_image_destroy(*imageStorage);
      *imageStorage = frame;
    }
  } else if (mDrawUtils == DrawUtils::BMCV) {
    bm_image_create(objectMetadata->mFrame->mHandle,
                    objectMetadata->mFrame->mHeight,
                    objectMetadata->mFrame->mWidth, FORMAT_YUV420P,
                    image.data_type, &(*imageStorage));
    bmcv_image_storage_convert(objectMetadata->mFrame->mHandle, 1, &image,
                               &(*imageStorage));
    switch (mOsdType) {
      case OsdType::DET:
        draw_bmcv_det_result(objectMetadata->mFrame->mHandle, objectMetadata,
                             mClassNames, *imageStorage, mPutText,
                             mDrawInterval);
        break;

      case OsdType::TRACK:
        draw_bmcv_track_result(objectMetadata->mFrame->mHandle, objectMetadata,
                               mClassNames, *imageStorage, mPutText,
                               mDrawInterval);
        break;

      case OsdType::POSE:
        draw_bmcv_pose_result(objectMetadata->mFrame->mHandle, objectMetadata,
                              *imageStorage, mDrawInterval);
        break;

      case OsdType::AREA:
        draw_bmcv_areas(objectMetadata, *imageStorage);
        break;

      case OsdType::TEXT:
        draw_text_results(objectMetadata, *imageStorage, overlay_image_, tops,
                          lefts, mDrawInterval);
        break;
      case OsdType::ALGORITHM:
        draw_func(objectMetadata, *imageStorage);
        break;
      default:
        IVS_WARN("osd_type not support");
    }
  } else {
  }

  objectMetadata->mFrame->mSpDataOsd = imageStorage;
}

REGISTER_WORKER("osd", Osd)

}  // namespace osd
}  // namespace element
}  // namespace sophon_stream
