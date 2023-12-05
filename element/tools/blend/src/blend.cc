//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//
#include "common/common_defs.h"
#if BMCV_VERSION_MAJOR > 1

#include <nlohmann/json.hpp>

#include "blend.h"
#include "common/logger.h"
#include "element_factory.h"

extern void bm_read_bin(bm_image src, const char* input_name);

void bm_dem_read_bin(bm_handle_t handle, bm_device_mem_t* dmem,
                     const char* input_name, unsigned int size) {
  char* input_ptr = (char*)malloc(size);
  FILE* fp_src = fopen(input_name, "rb+");

  if (fread((void*)input_ptr, 1, size, fp_src) < (unsigned int)size) {
    printf("file size is less than %d required bytes\n", size);
  };
  fclose(fp_src);

  if (BM_SUCCESS != bm_malloc_device_byte(handle, dmem, size)) {
    printf("bm_malloc_device_byte failed\n");
  }

  if (BM_SUCCESS != bm_memcpy_s2d(handle, *dmem, input_ptr)) {
    printf("bm_memcpy_s2d failed\n");
  }

  free(input_ptr);
  return;
}

namespace sophon_stream {
namespace element {
namespace blend {
Blend::Blend() {}
Blend::~Blend() {}

struct RequestDistTypeConfig {
  int type;
};

void to_json(nlohmann::json& j, const RequestDistTypeConfig& p) {
  j = nlohmann::json{{"type", p.type}};
}
void from_json(const nlohmann::json& j, RequestDistTypeConfig& p) {
  if (j.count("type")) p.type = j.at("type").get<int>();
}
bool str_to_object(const std::string& strjson, RequestDistTypeConfig& request) {
  nlohmann::json json_object = nlohmann::json::parse(strjson);
  request = json_object;
  return true;
}

void Blend::setDispType(const httplib::Request& request,
                        httplib::Response& response) {
  response.set_header("Access-Control-Allow-Origin", "*");
  response.set_header("Access-Control-Allow-Methods",
                      "GET, PATCH, PUT, OPTIONS");
  response.set_header("Access-Control-Allow-Headers",
                      "Content-Type, Authorization");
  if (request.method == "OPTIONS") {
    IVS_INFO("Display-Type Header meet");
    return;
  }
  common::Response resp;
  RequestDistTypeConfig rsi;
  IVS_INFO("HTTP REQUEST RECEIVED");
  str_to_object(request.body, rsi);
  {
    std::lock_guard<std::mutex> lk(mtx);
    dis_type = (DisplayType)rsi.type;
  }
  resp.msg = "success";
  nlohmann::json json_res = resp;
  response.set_content(json_res.dump(), "application/json");
  return;
}

void Blend::registListenFunc(sophon_stream::framework::ListenThread* listener) {
  std::string dispTypeStr = "/display-type-blend";

  listener->setHandler(dispTypeStr.c_str(),
                       sophon_stream::framework::RequestType::PUT,
                       std::bind(&Blend::setDispType, this,
                                 std::placeholders::_1, std::placeholders::_2));
  return;
}

common::ErrorCode Blend::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }

  bm_status_t ret = bm_dev_request(&handle, dev_id);

  src_h = 1080;
  // src_w = configure.find(CONFIG_INTERNAL_WEIGHT_FILED)->get<std::int>();
  auto wgt1 = configure.find(CONFIG_INTERNAL_WGT1_FILED)->get<std::string>();
  auto wgt2 = configure.find(CONFIG_INTERNAL_WGT2_FILED)->get<std::string>();
  char* wgt_name[2] = {NULL};
  wgt_name[0] = (char*)wgt1.c_str();
  wgt_name[1] = (char*)wgt2.c_str();

  memset(&blend_config, 0, sizeof(blend_config));
  blend_config.ovlap_attr.ovlp_lx[0] =
      configure.find(CONFIG_INTERNAL_OVLP_LX_FILED)->get<int>();
  blend_config.ovlap_attr.ovlp_rx[0] =
      configure.find(CONFIG_INTERNAL_OVLP_RX_FILED)->get<int>();
  blend_config.bd_attr.bd_lx[0] =
      configure.find(CONFIG_INTERNAL_BD_LX0_FILED)
          ->get<int>();  // left img, bd_attr from algo
  blend_config.bd_attr.bd_rx[0] =
      configure.find(CONFIG_INTERNAL_BD_RX0_FILED)->get<int>();
  blend_config.bd_attr.bd_lx[1] =
      configure.find(CONFIG_INTERNAL_BD_LX1_FILED)
          ->get<int>();  // right img, bd_attr from algo
  blend_config.bd_attr.bd_rx[1] =
      configure.find(CONFIG_INTERNAL_BD_RX1_FILED)->get<int>();

  blend_config.wgt_mode =
      (bm_stitch_wgt_mode)configure.find(CONFIG_INTERNAL_BD_RX1_FILED)
          ->get<int>();

  int wgtwidth = ALIGN(blend_config.ovlap_attr.ovlp_rx[0] -
                           blend_config.ovlap_attr.ovlp_lx[0] + 1,
                       16);
  int wgtheight = src_h;
  int wgt_len = wgtwidth * wgtheight;
  for (int i = 0; i < 2; i++) {
    bm_dem_read_bin(handle, &blend_config.wgt_phy_mem[0][i], wgt_name[i],
                    wgt_len);
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Blend::blend_work(
    std::shared_ptr<common::ObjectMetadata> leftObj,
    std::shared_ptr<common::ObjectMetadata> rightObj,
    std::shared_ptr<common::ObjectMetadata> blendObj) {
  std::lock_guard<std::mutex> lk(mtx);
  IVS_INFO("Now DisplayType is {0}", dis_type);
  std::shared_ptr<bm_image> blend_image = nullptr;
  blend_image.reset(new bm_image, [](bm_image* p) {
    bm_image_destroy(*p);
    delete p;
    p = nullptr;
  });
  bm_status_t ret =
      bm_image_create(handle, leftObj->mFrame->mSpData->height,
                      ALIGN(leftObj->mFrame->mSpData->width * 2 - 224, 32),
                      FORMAT_YUV420P, DATA_TYPE_EXT_1N_BYTE, blend_image.get());
  bm_image_alloc_dev_mem(*blend_image, 1);

  // select display type
  std::shared_ptr<bm_image> blend_image_left = nullptr;
  std::shared_ptr<bm_image> blend_image_right = nullptr;
  if (dis_type == ONLY_BLEND_DIS || dis_type == DWA_BLEND_DIS) {
    blend_image_left = leftObj->mFrame->mSpDataDwa;
    blend_image_right = rightObj->mFrame->mSpDataDwa;
  } else {
    blend_image_left = leftObj->mFrame->mSpData;
    blend_image_right = rightObj->mFrame->mSpData;
  }

  //
  bool need_convert = (blend_image_left->image_format != FORMAT_YUV420P ||
                       blend_image_right->image_format != FORMAT_YUV420P);

  bm_image blend_img[2];
  if (need_convert) {
    ret = bm_image_create(leftObj->mFrame->mHandle, blend_image_left->height,
                          blend_image_left->width, FORMAT_YUV420P,
                          DATA_TYPE_EXT_1N_BYTE, &blend_img[0], NULL);
    ret = bm_image_create(rightObj->mFrame->mHandle, blend_image_right->height,
                          blend_image_right->width, FORMAT_YUV420P,
                          DATA_TYPE_EXT_1N_BYTE, &blend_img[1], NULL);

    bm_image_alloc_dev_mem(blend_img[0], 1);
    bm_image_alloc_dev_mem(blend_img[1], 1);

    ret = bmcv_image_storage_convert(leftObj->mFrame->mHandle, 1,
                                     blend_image_left.get(), &blend_img[0]);
    ret = bmcv_image_storage_convert(rightObj->mFrame->mHandle, 1,
                                     blend_image_right.get(), &blend_img[1]);
  } else {
    blend_img[0] = *blend_image_left;
    blend_img[1] = *blend_image_right;
  }

  // blend

  ret = bmcv_blending(handle, input_num, blend_img, *blend_image, blend_config);

  // dispaly image
  if (dis_type != ONLY_BLEND_DIS) {
    std::shared_ptr<bm_image> all_image = nullptr;
    all_image.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    ret = bm_image_create(handle, blend_image_left->height * 3 + 200,
                          blend_image->width, FORMAT_YUV420P,
                          DATA_TYPE_EXT_1N_BYTE, all_image.get());
    bm_image_alloc_dev_mem(*all_image, 1);

    bm_device_mem_t blend_mem[3] = {0};
    bm_image_get_device_mem(*all_image, blend_mem);
    bm_memset_device(handle, 0, blend_mem[0]);
    bm_memset_device(handle, 0, blend_mem[1]);
    bm_memset_device(handle, 0, blend_mem[2]);

    bm_image src_img[3] = {blend_img[0], blend_img[1], *blend_image};

    // bmcv_rect_t rect = {0, 0, all_image->width, all_image->height};
    // ret = bmcv_image_fill_rectangle(handle, *all_image, 1, &rect, 0, 0, 0);

    int input_num = 3;
    bmcv_rect_t dst_crop[input_num];

    dst_crop[0] = {.start_x = 0,
                   .start_y = 0,
                   .crop_w = (unsigned int)blend_image_left->width,
                   .crop_h = (unsigned int)blend_image_left->height};

    dst_crop[1] = {.start_x = 0,
                   .start_y = (unsigned int)blend_image_right->height + 100,
                   .crop_w = (unsigned int)blend_image_right->width,
                   .crop_h = (unsigned int)blend_image_right->height};

    dst_crop[2] = {.start_x = 0,
                   .start_y = (unsigned int)blend_image_left->height +
                              (unsigned int)blend_image_right->height + 200,
                   .crop_w = (unsigned int)blend_image->width,
                   .crop_h = (unsigned int)blend_image->height};

    bmcv_image_vpp_stitch(handle, input_num, src_img, *all_image, dst_crop,
                          NULL);
    blendObj->mFrame->mSpData = all_image;
    blendObj->mFrame->mWidth = all_image->width;
    blendObj->mFrame->mHeight = all_image->height;
  } else {
    blendObj->mFrame->mSpData = blend_image;
    blendObj->mFrame->mWidth = blend_image->width;
    blendObj->mFrame->mHeight = blend_image->height;
  }

  blendObj->mFrame->mChannelId = leftObj->mFrame->mChannelId;
  blendObj->mFrame->mFrameId = leftObj->mFrame->mFrameId;
  blendObj->mFrame->mChannelIdInternal = leftObj->mFrame->mChannelIdInternal;
  blendObj->mFrame->mHandle = leftObj->mFrame->mHandle;
  
  if (need_convert) {
    bm_image_destroy(&blend_img[0]);
    bm_image_destroy(&blend_img[1]);
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Blend::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  common::ObjectMetadatas inputs;

  for (auto inputPort : inputPorts) {
    auto data = popInputData(inputPort, dataPipeId);
    while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      data = popInputData(inputPort, dataPipeId);
    }
    if (data == nullptr) return common::ErrorCode::SUCCESS;

    auto objectMetadata =
        std::static_pointer_cast<common::ObjectMetadata>(data);
    inputs.emplace_back(objectMetadata);
    IVS_INFO("Got Input, port id = {0}, channel_id = {1}, frame_id = {2}",
             inputPort, objectMetadata->mFrame->mChannelId,
             objectMetadata->mFrame->mFrameId);
  }

  if (inputs[0]->mFrame->mSpData != nullptr &&
      inputs[1]->mFrame->mSpData != nullptr) {
    std::shared_ptr<common::ObjectMetadata> blendObj =
        std::make_shared<common::ObjectMetadata>();

    blendObj->mFrame = std::make_shared<sophon_stream::common::Frame>();

    blend_work(inputs[0], inputs[1], blendObj);

    int channel_id_internal = blendObj->mFrame->mChannelIdInternal;
    int outDataPipeId =
        getSinkElementFlag()
            ? 0
            : (channel_id_internal % getOutputConnectorCapacity(outputPort));
    common::ErrorCode errorCode = pushOutputData(
        outputPort, outDataPipeId, std::static_pointer_cast<void>(blendObj));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(blendObj.get()));
    }
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("blend", Blend)
}  // namespace blend
}  // namespace element
}  // namespace sophon_stream

#endif
