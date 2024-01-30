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

#include <unordered_map>

#include "common/logger.h"
#include "element_factory.h"
#include "ive.h"
namespace sophon_stream {
namespace element {
namespace ive {
Ive::Ive() {}
Ive::~Ive() {}

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

void Ive::setDispType(const httplib::Request& request,
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

void Ive::registListenFunc(sophon_stream::framework::ListenThread* listener) {
  std::string dispTypeStr = "/display-type-dpu";

  listener->setHandler(dispTypeStr.c_str(),
                       sophon_stream::framework::RequestType::PUT,
                       std::bind(&Ive::setDispType, this, std::placeholders::_1,
                                 std::placeholders::_2));
  return;
}

common::ErrorCode Ive::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }

  mFpsProfiler.config("ive fps:", 100);

  bm_status_t ret = bm_dev_request(&handle, dev_id);

  is_ive = configure.find(CONFIG_INTERNAL_IS_IVE_FILED)->get<bool>();
  if (is_ive) {
    auto mapY_path =
        configure.find(CONFIG_INTERNAL_MAPY_FILED)->get<std::string>();
    auto mapU_path =
        configure.find(CONFIG_INTERNAL_MAPU_FILED)->get<std::string>();
    auto mapV_path =
        configure.find(CONFIG_INTERNAL_MAPV_FILED)->get<std::string>();

    FILE* fp;
    char str[256];
    int i = 0;
    fp = fopen(mapY_path.c_str(), "r");
    STREAM_CHECK(fp != NULL, "open map table failed!");
    while (fgets(str, 256, fp) != NULL) {
      FixMapY[i] = atoi(str);
      i++;
    }
    fclose(fp);
    fp = fopen(mapU_path.c_str(), "r");
    STREAM_CHECK(fp != NULL, "open map table failed!");
    i = 0;
    while (fgets(str, 256, fp) != NULL) {
      FixMapU[i] = atoi(str);
      i++;
    }
    fclose(fp);
    fp = fopen(mapV_path.c_str(), "r");
    STREAM_CHECK(fp != NULL, "open map table failed!");
    i = 0;
    while (fgets(str, 256, fp) != NULL) {
      FixMapV[i] = atoi(str);
      i++;
    }
    fclose(fp);

    bm_malloc_device_byte(handle, &mapTableY, MAP_TABLE_SIZE);
    ret = bm_memcpy_s2d(handle, mapTableY, FixMapY);
    if (ret != BM_SUCCESS) {
      IVS_DEBUG("bm_memcpy_s2d failed . ret = %d\n", ret);
      return common::ErrorCode::UNKNOWN;
    }

    bm_malloc_device_byte(handle, &mapTableU, MAP_TABLE_SIZE);
    ret = bm_memcpy_s2d(handle, mapTableU, FixMapU);
    if (ret != BM_SUCCESS) {
      IVS_DEBUG("bm_memcpy_s2d failed . ret = %d\n", ret);
      return common::ErrorCode::UNKNOWN;
    }

    bm_malloc_device_byte(handle, &mapTableV, MAP_TABLE_SIZE);
    ret = bm_memcpy_s2d(handle, mapTableV, FixMapV);
    if (ret != BM_SUCCESS) {
      IVS_DEBUG("bm_memcpy_s2d failed . ret = %d\n", ret);
      return common::ErrorCode::UNKNOWN;
    }

    map_mode = IVE_MAP_U8;
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Ive::ive_work(
    std::shared_ptr<common::ObjectMetadata> iveObj) {
  bm_status_t ret;
  if (is_ive == true) {
    auto start = std::chrono::high_resolution_clock::now();

    std::shared_ptr<bm_image> dpu_image_map = nullptr;
    dpu_image_map.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    int ive_src_stride[4];
    bm_ive_image_calc_stride(iveObj->mFrame->mHandle,
                             iveObj->mFrame->mSpDataDpu->height,
                             iveObj->mFrame->mSpDataDpu->width, FORMAT_YUV444P,
                             DATA_TYPE_EXT_1N_BYTE, ive_src_stride);
    bm_image_create(handle, iveObj->mFrame->mSpDataDpu->height,
                    iveObj->mFrame->mSpDataDpu->width, FORMAT_YUV444P,
                    DATA_TYPE_EXT_1N_BYTE, dpu_image_map.get(), ive_src_stride);

    bm_image_alloc_dev_mem(*dpu_image_map, 1);
    bm_image dpu_image_map_y;
    bm_image dpu_image_map_u;
    bm_image dpu_image_map_v;

    bm_image_create(iveObj->mFrame->mHandle, dpu_image_map->height,
                    dpu_image_map->width, FORMAT_GRAY, DATA_TYPE_EXT_1N_BYTE,
                    &dpu_image_map_y, ive_src_stride);
    bm_image_create(iveObj->mFrame->mHandle, dpu_image_map->height,
                    dpu_image_map->width, FORMAT_GRAY, DATA_TYPE_EXT_1N_BYTE,
                    &dpu_image_map_u, ive_src_stride);
    bm_image_create(iveObj->mFrame->mHandle, dpu_image_map->height,
                    dpu_image_map->width, FORMAT_GRAY, DATA_TYPE_EXT_1N_BYTE,
                    &dpu_image_map_v, ive_src_stride);

    // output
    bm_device_mem_t dpu_image_map_mem[3] = {0};
    bm_image_get_device_mem(*dpu_image_map, dpu_image_map_mem);
    bm_image_attach(dpu_image_map_y, &dpu_image_map_mem[0]);  // y
    bm_image_attach(dpu_image_map_u, &dpu_image_map_mem[1]);  // uv
    bm_image_attach(dpu_image_map_v, &dpu_image_map_mem[2]);  // uv

    // ive
    bmcv_image_ive_map(handle, map_mode, mapTableY,
                       iveObj->mFrame->mSpDataDpu.get(), &dpu_image_map_y);
    bmcv_image_ive_map(handle, map_mode, mapTableU,
                       iveObj->mFrame->mSpDataDpu.get(), &dpu_image_map_u);
    bmcv_image_ive_map(handle, map_mode, mapTableV,
                       iveObj->mFrame->mSpDataDpu.get(), &dpu_image_map_v);

    // bm_image_destroy
    bm_image_destroy(&dpu_image_map_y);
    bm_image_destroy(&dpu_image_map_u);
    bm_image_destroy(&dpu_image_map_v);

    iveObj->mFrame->mSpDataDpu = dpu_image_map;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "dpu_ive_map程序执行时间：" << duration.count() << " ms"
              << std::endl;
  }

  auto start = std::chrono::high_resolution_clock::now();
  std::shared_ptr<bm_image> stitch_image = nullptr;
  if (dis_type != ONLY_DPU_DIS) {
    stitch_image.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    ret = bm_image_create(handle, iveObj->mFrame->mSpDataDpu->height,
                          iveObj->mFrame->mSpDataDpu->width * 2 + 0,
                          FORMAT_YUV444P, DATA_TYPE_EXT_1N_BYTE,
                          stitch_image.get());
    bm_image_alloc_dev_mem(*stitch_image, 1);

    bm_device_mem_t dpu_mem[3];
    bm_image_get_device_mem(*stitch_image, dpu_mem);
    bm_memset_device(handle, 0, dpu_mem[0]);
    bm_memset_device(handle, 0, dpu_mem[1]);
    bm_memset_device(handle, 0, dpu_mem[2]);

    int input_num = 2;
    bmcv_rect_t dst_crop[input_num];

    dst_crop[0] = {.start_x = 0,
                   .start_y = 0,
                   .crop_w = (unsigned int)iveObj->mFrame->mSpDataDpu->width,
                   .crop_h = (unsigned int)iveObj->mFrame->mSpDataDpu->height};
    dst_crop[1] = {
        .start_x = (unsigned int)iveObj->mFrame->mSpDataDpu->width * 1 + 0,
        .start_y = 0,
        .crop_w = (unsigned int)iveObj->mFrame->mSpDataDpu->width,
        .crop_h = (unsigned int)iveObj->mFrame->mSpDataDpu->height};
    if (dis_type == RAW_DPU_DIS) {
      bm_image src_img[2] = {*iveObj->mFrame->mSpData,
                             *iveObj->mFrame->mSpDataDpu};
      bmcv_image_vpp_stitch(handle, input_num, src_img, *stitch_image, dst_crop,
                            NULL);
    } else {
      bm_image src_img[2] = {*iveObj->mFrame->mSpDataDwa,
                             *iveObj->mFrame->mSpDataDpu};
      bmcv_image_vpp_stitch(handle, input_num, src_img, *stitch_image, dst_crop,
                            NULL);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "bmcv_image_vpp_stitch程序执行时间：" << duration.count()
              << " ms" << std::endl;

    // bm_image_write_to_bmp(*iveObj->mFrame->mSpDataDpu,"dpumSpData.bmp");
    // bm_image_write_to_bmp(*stitch_image,"stitch_image.bmp");
    iveObj->mFrame->mSpData = stitch_image;
  } else {
    iveObj->mFrame->mSpData = iveObj->mFrame->mSpDataDpu;
  }

  iveObj->mFrame->mWidth = iveObj->mFrame->mSpData->width;
  iveObj->mFrame->mHeight = iveObj->mFrame->mSpData->height;

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Ive::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    int outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  if (objectMetadata->mFrame != nullptr &&
      objectMetadata->mFrame->mSpData != nullptr) {
    auto start = std::chrono::high_resolution_clock::now();
    ive_work(objectMetadata);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "ive_work程序执行时间：" << duration.count() << " ms"
              << std::endl;
  }

  mFpsProfiler.add(1);
  // usleep(10);
  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  common::ErrorCode errorCode =
      pushOutputData(outputPort, outDataPipeId,
                     std::static_pointer_cast<void>(objectMetadata));
  if (common::ErrorCode::SUCCESS != errorCode) {
    IVS_WARN(
        "Send data fail, element id: {0:d}, output port: {1:d}, data: "
        "{2:p}",
        getId(), outputPort, static_cast<void*>(objectMetadata.get()));
  }
  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("ive", Ive)
}  // namespace ive
}  // namespace element
}  // namespace sophon_stream

#endif