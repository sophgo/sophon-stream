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
#include "dpu.h"
#include "element_factory.h"
namespace sophon_stream {
namespace element {
namespace dpu {
Dpu::Dpu() {}
Dpu::~Dpu() {}

bm_status_t set_sgbm_default_param(bmcv_dpu_sgbm_attrs* grp_) {
  grp_->bfw_mode_en = DPU_BFW_MODE_7x7;
  grp_->disp_range_en = BMCV_DPU_DISP_RANGE_128;
  grp_->disp_start_pos = 0;
  grp_->dcc_dir_en = BMCV_DPU_DCC_DIR_A14;
  grp_->dpu_census_shift = 1;
  grp_->dpu_rshift1 = 0;
  grp_->dpu_rshift2 = 2;
  grp_->dpu_ca_p1 = 1800;
  grp_->dpu_ca_p2 = 14400;
  grp_->dpu_uniq_ratio = 25;
  grp_->dpu_disp_shift = 4;
  return BM_SUCCESS;
}

bm_status_t set_online_default_param(bmcv_dpu_sgbm_attrs* grp_,
                                     bmcv_dpu_fgs_attrs* fgs_grp) {
  grp_->bfw_mode_en = DPU_BFW_MODE_7x7;
  grp_->disp_range_en = BMCV_DPU_DISP_RANGE_16;
  grp_->disp_start_pos = 0;
  grp_->dcc_dir_en = BMCV_DPU_DCC_DIR_A12;
  grp_->dpu_census_shift = 1;
  grp_->dpu_rshift1 = 3;
  grp_->dpu_rshift2 = 2;
  grp_->dpu_ca_p1 = 1800;
  grp_->dpu_ca_p2 = 14400;
  grp_->dpu_uniq_ratio = 25;
  grp_->dpu_disp_shift = 4;
  fgs_grp->depth_unit_en = BMCV_DPU_DEPTH_UNIT_MM;
  fgs_grp->fgs_max_count = 19;
  fgs_grp->fgs_max_t = 2;
  fgs_grp->fxbase_line = 864000;
  return BM_SUCCESS;
}

// get请求没有内容
struct RequestGetConfig {};
void to_json(nlohmann::json& j, const RequestGetConfig& p) {}
void from_json(const nlohmann::json& j, RequestGetConfig& p) {}
bool str_to_object(const std::string& strjson, RequestGetConfig& request) {
  return true;
}

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

struct RequestSetConfig {
  bmcv_dpu_online_mode DpuMode;
  bmcv_dpu_bfw_mode MaskMode;
  bmcv_dpu_disp_range DispRange;
  unsigned short DispStartPos;
  bmcv_dpu_dcc_dir DccDir;
  unsigned int CensusShift;
  unsigned int Rshift1;
  unsigned int Rshift2;
  unsigned int CaP1;
  unsigned int CaP2;
  unsigned int UniqRatio;
  unsigned int DispShift;
  bmcv_dpu_depth_unit DepthUnit;
  unsigned int MaxCount;
  unsigned int MaxT;
  unsigned int FxbaseLine;
};

std::unordered_map<std::string, bmcv_dpu_online_mode> DpuModeMap = {
    {"DPU_ONLINE_MUX0", DPU_ONLINE_MUX0},
    {"DPU_ONLINE_MUX1", DPU_ONLINE_MUX1},
    {"DPU_ONLINE_MUX2", DPU_ONLINE_MUX2},
    {"DEFAULT", DPU_ONLINE_MUX0},
    {"", DPU_ONLINE_MUX0}};

std::unordered_map<bmcv_dpu_online_mode, std::string> rDpuModeMap = {
    {DPU_ONLINE_MUX0, "DPU_ONLINE_MUX0"},
    {DPU_ONLINE_MUX1, "DPU_ONLINE_MUX1"},
    {DPU_ONLINE_MUX2, "DPU_ONLINE_MUX2"}};

std::unordered_map<std::string, bmcv_dpu_bfw_mode> MaskModeMap = {
    {"DPU_BFW_MODE_DEFAULT", DPU_BFW_MODE_DEFAULT},
    {"DPU_BFW_MODE_1x1", DPU_BFW_MODE_1x1},
    {"DPU_BFW_MODE_3x3", DPU_BFW_MODE_3x3},
    {"DPU_BFW_MODE_5x5", DPU_BFW_MODE_5x5},
    {"DPU_BFW_MODE_7x7", DPU_BFW_MODE_7x7},
    {"DPU_BFW_MODE_BUTT", DPU_BFW_MODE_BUTT},
    {"", DPU_BFW_MODE_DEFAULT}};

std::unordered_map<bmcv_dpu_bfw_mode, std::string> rMaskModeMap = {
    {DPU_BFW_MODE_DEFAULT, "DPU_BFW_MODE_DEFAULT"},
    {DPU_BFW_MODE_1x1, "DPU_BFW_MODE_1x1"},
    {DPU_BFW_MODE_3x3, "DPU_BFW_MODE_3x3"},
    {DPU_BFW_MODE_5x5, "DPU_BFW_MODE_5x5"},
    {DPU_BFW_MODE_7x7, "DPU_BFW_MODE_7x7"},
    {DPU_BFW_MODE_BUTT, "DPU_BFW_MODE_BUTT"}};

std::unordered_map<std::string, bmcv_dpu_disp_range> DispRangeMap = {
    {"BMCV_DPU_DISP_RANGE_DEFAULT", BMCV_DPU_DISP_RANGE_DEFAULT},
    {"BMCV_DPU_DISP_RANGE_16", BMCV_DPU_DISP_RANGE_16},
    {"BMCV_DPU_DISP_RANGE_32", BMCV_DPU_DISP_RANGE_32},
    {"BMCV_DPU_DISP_RANGE_48", BMCV_DPU_DISP_RANGE_48},
    {"BMCV_DPU_DISP_RANGE_64", BMCV_DPU_DISP_RANGE_64},
    {"BMCV_DPU_DISP_RANGE_80", BMCV_DPU_DISP_RANGE_80},
    {"BMCV_DPU_DISP_RANGE_96", BMCV_DPU_DISP_RANGE_96},
    {"BMCV_DPU_DISP_RANGE_112", BMCV_DPU_DISP_RANGE_112},
    {"BMCV_DPU_DISP_RANGE_128", BMCV_DPU_DISP_RANGE_128},
    {"BMCV_DPU_DISP_RANGE_BUTT", BMCV_DPU_DISP_RANGE_BUTT},
    {"", BMCV_DPU_DISP_RANGE_DEFAULT}};

std::unordered_map<bmcv_dpu_disp_range, std::string> rDispRangeMap = {
    {BMCV_DPU_DISP_RANGE_DEFAULT, "BMCV_DPU_DISP_RANGE_DEFAULT"},
    {BMCV_DPU_DISP_RANGE_16, "BMCV_DPU_DISP_RANGE_16"},
    {BMCV_DPU_DISP_RANGE_32, "BMCV_DPU_DISP_RANGE_32"},
    {BMCV_DPU_DISP_RANGE_48, "BMCV_DPU_DISP_RANGE_48"},
    {BMCV_DPU_DISP_RANGE_64, "BMCV_DPU_DISP_RANGE_64"},
    {BMCV_DPU_DISP_RANGE_80, "BMCV_DPU_DISP_RANGE_80"},
    {BMCV_DPU_DISP_RANGE_96, "BMCV_DPU_DISP_RANGE_96"},
    {BMCV_DPU_DISP_RANGE_112, "BMCV_DPU_DISP_RANGE_112"},
    {BMCV_DPU_DISP_RANGE_128, "BMCV_DPU_DISP_RANGE_128"},
    {BMCV_DPU_DISP_RANGE_BUTT, "BMCV_DPU_DISP_RANGE_BUTT"}};

std::unordered_map<std::string, bmcv_dpu_depth_unit> DepthUnitMap = {
    {"BMCV_DPU_DEPTH_UNIT_DEFAULT", BMCV_DPU_DEPTH_UNIT_DEFAULT},
    {"BMCV_DPU_DEPTH_UNIT_MM", BMCV_DPU_DEPTH_UNIT_MM},
    {"BMCV_DPU_DEPTH_UNIT_CM", BMCV_DPU_DEPTH_UNIT_CM},
    {"BMCV_DPU_DEPTH_UNIT_DM", BMCV_DPU_DEPTH_UNIT_DM},
    {"BMCV_DPU_DEPTH_UNIT_M", BMCV_DPU_DEPTH_UNIT_M},
    {"BMCV_DPU_DEPTH_UNIT_BUTT", BMCV_DPU_DEPTH_UNIT_BUTT},
    {"", BMCV_DPU_DEPTH_UNIT_DEFAULT}};

std::unordered_map<bmcv_dpu_depth_unit, std::string> rDepthUnitMap = {
    {BMCV_DPU_DEPTH_UNIT_DEFAULT, "BMCV_DPU_DEPTH_UNIT_DEFAULT"},
    {BMCV_DPU_DEPTH_UNIT_MM, "BMCV_DPU_DEPTH_UNIT_MM"},
    {BMCV_DPU_DEPTH_UNIT_CM, "BMCV_DPU_DEPTH_UNIT_CM"},
    {BMCV_DPU_DEPTH_UNIT_DM, "BMCV_DPU_DEPTH_UNIT_DM"},
    {BMCV_DPU_DEPTH_UNIT_M, "BMCV_DPU_DEPTH_UNIT_M"},
    {BMCV_DPU_DEPTH_UNIT_BUTT, "BMCV_DPU_DEPTH_UNIT_BUTT"}};

std::unordered_map<std::string, bmcv_dpu_dcc_dir> DccDirMap = {
    {"BMCV_DPU_DCC_DIR_DEFAULT", BMCV_DPU_DCC_DIR_DEFAULT},
    {"BMCV_DPU_DCC_DIR_A12", BMCV_DPU_DCC_DIR_A12},
    {"BMCV_DPU_DCC_DIR_A13", BMCV_DPU_DCC_DIR_A13},
    {"BMCV_DPU_DCC_DIR_A14", BMCV_DPU_DCC_DIR_A14},
    {"BMCV_DPU_DCC_DIR_BUTT", BMCV_DPU_DCC_DIR_BUTT},
    {"", BMCV_DPU_DCC_DIR_DEFAULT}};

std::unordered_map<bmcv_dpu_dcc_dir, std::string> rDccDirMap = {
    {BMCV_DPU_DCC_DIR_DEFAULT, "BMCV_DPU_DCC_DIR_DEFAULT"},
    {BMCV_DPU_DCC_DIR_A12, "BMCV_DPU_DCC_DIR_A12"},
    {BMCV_DPU_DCC_DIR_A13, "BMCV_DPU_DCC_DIR_A13"},
    {BMCV_DPU_DCC_DIR_A14, "BMCV_DPU_DCC_DIR_A14"},
    {BMCV_DPU_DCC_DIR_BUTT, "BMCV_DPU_DCC_DIR_BUTT"}};

void to_json(nlohmann::json& j, const RequestSetConfig& p) {}
void from_json(const nlohmann::json& j, RequestSetConfig& p) {
  if (j.count("DpuMode")) {
    p.DpuMode = DpuModeMap[j.at("DpuMode").get<std::string>()];
  }
  if (j.count("MaskMode")) {
    p.MaskMode = MaskModeMap[j.at("MaskMode").get<std::string>()];
  }
  if (j.count("DispRange")) {
    p.DispRange = DispRangeMap[j.at("DispRange").get<std::string>()];
  }
  if (j.count("DispStartPos")) {
    p.DispStartPos = std::stoi(j.at("DispStartPos").get<std::string>());
  }
  if (j.count("DccDir")) {
    p.DccDir = DccDirMap[j.at("DccDir").get<std::string>()];
  }
  if (j.count("CensusShift")) {
    p.CensusShift = std::stoi(j.at("CensusShift").get<std::string>());
  }
  if (j.count("Rshift1")) {
    p.Rshift1 = std::stoi(j.at("Rshift1").get<std::string>());
  }
  if (j.count("Rshift2")) {
    p.Rshift2 = std::stoi(j.at("Rshift2").get<std::string>());
  }
  if (j.count("CaP1")) {
    p.CaP1 = std::stoi(j.at("CaP1").get<std::string>());
  }
  if (j.count("CaP2")) {
    p.CaP2 = std::stoi(j.at("CaP2").get<std::string>());
  }
  if (j.count("UniqRatio")) {
    p.UniqRatio = std::stoi(j.at("UniqRatio").get<std::string>());
  }
  if (j.count("DispShift")) {
    p.DispShift = std::stoi(j.at("DispShift").get<std::string>());
  }
  if (j.count("DepthUnit")) {
    p.DepthUnit = DepthUnitMap[j.at("DepthUnit").get<std::string>()];
  }
  if (j.count("MaxCount")) {
    p.MaxCount = std::stoi(j.at("MaxCount").get<std::string>());
  }
  if (j.count("MaxT")) {
    p.MaxT = std::stoi(j.at("MaxT").get<std::string>());
  }
  if (j.count("FxbaseLine")) {
    p.FxbaseLine = std::stoi(j.at("FxbaseLine").get<std::string>());
  }
}
bool str_to_object(const std::string& strjson, RequestSetConfig& request) {
  nlohmann::json json_object = nlohmann::json::parse(strjson);
  request = json_object;
  return true;
}

struct ResponseGetConfig {
  std::unordered_map<std::string, std::string> rgcMap;
};

void to_json(nlohmann::json& j, const ResponseGetConfig& p) {
  for (auto& [k, v] : p.rgcMap) {
    j["data"][k] = v;
  }
  return;
}

void Dpu::getConfig(const httplib::Request& request,
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
  ResponseGetConfig resp;
  {
    std::lock_guard<std::mutex> lk(mtx);
    resp.rgcMap["DpuMode"] = rDpuModeMap[dpu_mode];
    resp.rgcMap["MaskMode"] = rMaskModeMap[dpu_sgbm_attr.bfw_mode_en];
    resp.rgcMap["DispRange"] = rDispRangeMap[dpu_sgbm_attr.disp_range_en];
    resp.rgcMap["DispStartPos"] = std::to_string(dpu_sgbm_attr.disp_start_pos);
    resp.rgcMap["DccDir"] = rDccDirMap[dpu_sgbm_attr.dcc_dir_en];
    resp.rgcMap["CensusShift"] = std::to_string(dpu_sgbm_attr.dpu_census_shift);
    resp.rgcMap["Rshift1"] = std::to_string(dpu_sgbm_attr.dpu_rshift1);
    resp.rgcMap["Rshift2"] = std::to_string(dpu_sgbm_attr.dpu_rshift2);
    resp.rgcMap["CaP1"] = std::to_string(dpu_sgbm_attr.dpu_ca_p1);
    resp.rgcMap["CaP2"] = std::to_string(dpu_sgbm_attr.dpu_ca_p2);
    resp.rgcMap["UniqRatio"] = std::to_string(dpu_sgbm_attr.dpu_uniq_ratio);
    resp.rgcMap["DispShift"] = std::to_string(dpu_sgbm_attr.dpu_disp_shift);

    resp.rgcMap["DepthUnit"] = rDepthUnitMap[dpu_fgs_attr.depth_unit_en];
    resp.rgcMap["MaxCount"] = std::to_string(dpu_fgs_attr.fgs_max_count);
    resp.rgcMap["MaxT"] = std::to_string(dpu_fgs_attr.fgs_max_t);
    resp.rgcMap["FxbaseLine"] = std::to_string(dpu_fgs_attr.fxbase_line);
  }
  nlohmann::json json_res = resp;
  response.set_content(json_res.dump(), "application/json");
  return;
}

void Dpu::setConfig(const httplib::Request& request,
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
  RequestSetConfig rsc;
  str_to_object(request.body, rsc);
  // 这里从json取出了各个需要set的属性，然后做赋值
  {
    std::lock_guard<std::mutex> lk(mtx);
    dpu_mode = rsc.DpuMode;
    dpu_sgbm_attr.bfw_mode_en = rsc.MaskMode;
    dpu_sgbm_attr.disp_range_en = rsc.DispRange;
    dpu_sgbm_attr.disp_start_pos = rsc.DispStartPos;
    dpu_sgbm_attr.dcc_dir_en = rsc.DccDir;
    dpu_sgbm_attr.dpu_census_shift = rsc.CensusShift;
    dpu_sgbm_attr.dpu_rshift1 = rsc.Rshift1;
    dpu_sgbm_attr.dpu_rshift2 = rsc.Rshift2;
    dpu_sgbm_attr.dpu_ca_p1 = rsc.CaP1;
    dpu_sgbm_attr.dpu_ca_p2 = rsc.CaP2;
    dpu_sgbm_attr.dpu_uniq_ratio = rsc.UniqRatio;
    dpu_sgbm_attr.dpu_disp_shift = rsc.DispShift;

    dpu_fgs_attr.depth_unit_en = rsc.DepthUnit;
    dpu_fgs_attr.fgs_max_count = rsc.MaxCount;
    dpu_fgs_attr.fgs_max_t = rsc.MaxT;
    dpu_fgs_attr.fxbase_line = rsc.FxbaseLine;
  }
  // 设置参数完成
  ResponseGetConfig resp;
  {
    std::lock_guard<std::mutex> lk(mtx);
    resp.rgcMap["DpuMode"] = rDpuModeMap[dpu_mode];
    resp.rgcMap["MaskMode"] = rMaskModeMap[dpu_sgbm_attr.bfw_mode_en];
    resp.rgcMap["DispRange"] = rDispRangeMap[dpu_sgbm_attr.disp_range_en];
    resp.rgcMap["DispStartPos"] = std::to_string(dpu_sgbm_attr.disp_start_pos);
    resp.rgcMap["DccDir"] = rDccDirMap[dpu_sgbm_attr.dcc_dir_en];
    resp.rgcMap["CensusShift"] = std::to_string(dpu_sgbm_attr.dpu_census_shift);
    resp.rgcMap["Rshift1"] = std::to_string(dpu_sgbm_attr.dpu_rshift1);
    resp.rgcMap["Rshift2"] = std::to_string(dpu_sgbm_attr.dpu_rshift2);
    resp.rgcMap["CaP1"] = std::to_string(dpu_sgbm_attr.dpu_ca_p1);
    resp.rgcMap["CaP2"] = std::to_string(dpu_sgbm_attr.dpu_ca_p2);
    resp.rgcMap["UniqRatio"] = std::to_string(dpu_sgbm_attr.dpu_uniq_ratio);
    resp.rgcMap["DispShift"] = std::to_string(dpu_sgbm_attr.dpu_disp_shift);

    resp.rgcMap["DepthUnit"] = rDepthUnitMap[dpu_fgs_attr.depth_unit_en];
    resp.rgcMap["MaxCount"] = std::to_string(dpu_fgs_attr.fgs_max_count);
    resp.rgcMap["MaxT"] = std::to_string(dpu_fgs_attr.fgs_max_t);
    resp.rgcMap["FxbaseLine"] = std::to_string(dpu_fgs_attr.fxbase_line);
  }
  nlohmann::json json_res = resp;
  response.set_content(json_res.dump(), "application/json");
  return;
}

void Dpu::setDispType(const httplib::Request& request,
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
  dis_type = (DisplayType)rsi.type;
  resp.msg = "success";
  nlohmann::json json_res = resp;
  response.set_content(json_res.dump(), "application/json");
  return;
}

void Dpu::registListenFunc(sophon_stream::framework::ListenThread* listener) {
  // std::string getConfigStr = "/dpu/getConfig/" + std::to_string(getId());
  // std::string setConfigStr = "/dpu/setConfig/" + std::to_string(getId());

  std::string getConfigStr = "/config";
  std::string setConfigStr = "/config";
  std::string dispTypeStr = "/display-type-dpu";

  listener->setHandler(getConfigStr.c_str(),
                       sophon_stream::framework::RequestType::GET,
                       std::bind(&Dpu::getConfig, this, std::placeholders::_1,
                                 std::placeholders::_2));
  listener->setHandler(setConfigStr.c_str(),
                       sophon_stream::framework::RequestType::PUT,
                       std::bind(&Dpu::setConfig, this, std::placeholders::_1,
                                 std::placeholders::_2));
  listener->setHandler(dispTypeStr.c_str(),
                       sophon_stream::framework::RequestType::PUT,
                       std::bind(&Dpu::setDispType, this, std::placeholders::_1,
                                 std::placeholders::_2));
  return;
}

common::ErrorCode Dpu::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }

  bm_status_t ret = bm_dev_request(&handle, dev_id);

  dpu_mode = DPU_ONLINE_MUX0;
  // set_sgbm_default_param(&dpu_sgbm_attr);
  set_online_default_param(&dpu_sgbm_attr, &dpu_fgs_attr);

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
  STREAM_CHECK(fp != NULL,"open map table failed!");
  while (fgets(str, 256, fp) != NULL) {
    FixMapY[i] = atoi(str);
    i++;
  }
  fclose(fp);
  fp = fopen(mapU_path.c_str(), "r");
  STREAM_CHECK(fp != NULL,"open map table failed!");
  i = 0;
  while (fgets(str, 256, fp) != NULL) {
    FixMapU[i] = atoi(str);
    i++;
  }
  fclose(fp);
  fp = fopen(mapV_path.c_str(), "r");
  STREAM_CHECK(fp != NULL,"open map table failed!");
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

  return common::ErrorCode::SUCCESS;
}

void Dpu::dpu_ive_map(bm_image& dpu_image, bm_image& dpu_image_map,
                      int ive_src_stride[]) {
  bm_ive_image_calc_stride(handle, dpu_image_map.height, dpu_image_map.width,
                           FORMAT_YUV444P, DATA_TYPE_EXT_1N_BYTE,
                           ive_src_stride);
  bm_image dpu_image_map_y;
  bm_image dpu_image_map_u;
  bm_image dpu_image_map_v;

  bm_image_create(handle, dpu_image_map.height, dpu_image_map.width, dpu_fmt,
                  DATA_TYPE_EXT_1N_BYTE, &dpu_image_map_y, ive_src_stride);
  bm_image_create(handle, dpu_image_map.height, dpu_image_map.width, dpu_fmt,
                  DATA_TYPE_EXT_1N_BYTE, &dpu_image_map_u, ive_src_stride);
  bm_image_create(handle, dpu_image_map.height, dpu_image_map.width, dpu_fmt,
                  DATA_TYPE_EXT_1N_BYTE, &dpu_image_map_v, ive_src_stride);

  // output
  bm_device_mem_t dpu_image_map_mem[3] = {0};
  bm_image_get_device_mem(dpu_image_map, dpu_image_map_mem);
  bm_image_attach(dpu_image_map_y, &dpu_image_map_mem[0]);  // y
  bm_image_attach(dpu_image_map_u, &dpu_image_map_mem[1]);  // uv
  bm_image_attach(dpu_image_map_v, &dpu_image_map_mem[2]);  // uv

  // ive
  bmcv_image_ive_map(handle, map_mode, mapTableY, &dpu_image, &dpu_image_map_y);
  bmcv_image_ive_map(handle, map_mode, mapTableU, &dpu_image, &dpu_image_map_u);
  bmcv_image_ive_map(handle, map_mode, mapTableV, &dpu_image, &dpu_image_map_v);

  // bm_image_destroy
  bm_image_destroy(&dpu_image_map_y);
  bm_image_destroy(&dpu_image_map_u);
  bm_image_destroy(&dpu_image_map_v);
}

common::ErrorCode Dpu::dpu_work(
    std::shared_ptr<common::ObjectMetadata> leftObj,
    std::shared_ptr<common::ObjectMetadata> rightObj,
    std::shared_ptr<common::ObjectMetadata> dpuObj) {
  bm_status_t ret;
  // select display type
  std::shared_ptr<bm_image> dpu_image_left = nullptr;
  std::shared_ptr<bm_image> dpu_image_right = nullptr;
  if (dis_type == ONLY_DPU_DIS || dis_type == DWA_DPU_DIS) {
    dpu_image_left = leftObj->mFrame->mSpDataDwa;
    dpu_image_right = rightObj->mFrame->mSpDataDwa;
  } else {
    dpu_image_left = leftObj->mFrame->mSpData;
    dpu_image_right = rightObj->mFrame->mSpData;
  }

  // dpu输出结果
  bm_image dpu_out;
  bm_image_create(handle, dpu_image_left->height, dpu_image_left->width,
                  dpu_fmt, DATA_TYPE_EXT_1N_BYTE, &dpu_out);
  bm_image_alloc_dev_mem(dpu_out, BMCV_HEAP_ANY);

  // ive_map结果
  std::shared_ptr<bm_image> dpu_image_map = nullptr;
  dpu_image_map.reset(new bm_image, [](bm_image* p) {
    bm_image_destroy(*p);
    delete p;
    p = nullptr;
  });

  int ive_src_stride[4];
  bm_ive_image_calc_stride(handle, dpu_image_left->height,
                           dpu_image_left->width, FORMAT_YUV444P,
                           DATA_TYPE_EXT_1N_BYTE, ive_src_stride);
  bm_image_create(handle, leftObj->mFrame->mSpData->height,
                  leftObj->mFrame->mSpData->width, FORMAT_YUV444P,
                  DATA_TYPE_EXT_1N_BYTE, dpu_image_map.get(), ive_src_stride);

  bm_image_alloc_dev_mem(*dpu_image_map, 1);

  //
  bool need_convert = (dpu_image_left->image_format != dpu_fmt ||
                       dpu_image_right->image_format != dpu_fmt);
  bm_image dpu_img[2];
  if (need_convert) {
    bm_image_create(leftObj->mFrame->mHandle, dpu_image_left->height,
                          dpu_image_left->width, dpu_fmt, DATA_TYPE_EXT_1N_BYTE,
                          &dpu_img[0], NULL);
    bm_image_create(rightObj->mFrame->mHandle, dpu_image_right->height,
                          dpu_image_right->width, dpu_fmt,
                          DATA_TYPE_EXT_1N_BYTE, &dpu_img[1], NULL);

    bm_image_alloc_dev_mem(dpu_img[0], 1);
    bm_image_alloc_dev_mem(dpu_img[1], 1);

    bmcv_image_storage_convert(leftObj->mFrame->mHandle, 1,
                                     dpu_image_left.get(), &dpu_img[0]);
    bmcv_image_storage_convert(rightObj->mFrame->mHandle, 1,
                                     dpu_image_right.get(), &dpu_img[1]);
  } else {
    dpu_img[0] = *dpu_image_left;
    dpu_img[1] = *dpu_image_right;
  }

  // dpu and ive map
  {
    std::lock_guard<std::mutex> lk(mtx);
    bmcv_dpu_online_disp(handle, &dpu_img[0], &dpu_img[1], &dpu_out,
                               &dpu_sgbm_attr, &dpu_fgs_attr, dpu_mode);
    dpu_ive_map(dpu_out, *dpu_image_map, ive_src_stride);
  }

  // dispaly image
  if (dis_type != ONLY_DPU_DIS) {
    std::shared_ptr<bm_image> all_image = nullptr;
    all_image.reset(new bm_image, [](bm_image* p) {
      bm_image_destroy(*p);
      delete p;
      p = nullptr;
    });
    ret = bm_image_create(handle, dpu_image_map->height,
                          dpu_image_map->width *2+100, FORMAT_YUV444P,
                          DATA_TYPE_EXT_1N_BYTE, all_image.get());
    bm_image_alloc_dev_mem(*all_image, 1);

    bm_device_mem_t dpu_mem[3];
    bm_image_get_device_mem(*all_image, dpu_mem);
    bm_memset_device(handle, 0, dpu_mem[0]);
    bm_memset_device(handle, 0, dpu_mem[1]);
    bm_memset_device(handle, 0, dpu_mem[2]);

    int input_num = 2;
    bmcv_rect_t dst_crop[input_num];

    dst_crop[0] = {.start_x = 0,
                   .start_y = 0,
                   .crop_w = (unsigned int)dpu_image_left->width,
                   .crop_h = (unsigned int)dpu_image_left->height};

    dst_crop[1] = {.start_x = (unsigned int)dpu_image_left->width + 100,
                   .start_y = 0,
                   .crop_w = (unsigned int)dpu_image_map->width,
                   .crop_h = (unsigned int)dpu_image_map->height};

    bm_image src_img[2] = {dpu_img[0], *dpu_image_map};

    bmcv_image_vpp_stitch(handle, input_num, src_img, *all_image, dst_crop,
                          NULL);

    dpuObj->mFrame->mSpData = all_image;
    dpuObj->mFrame->mWidth = all_image->width;
    dpuObj->mFrame->mHeight = all_image->height;
  } else {
    dpuObj->mFrame->mSpData = dpu_image_map;
    dpuObj->mFrame->mWidth = dpu_image_map->width;
    dpuObj->mFrame->mHeight = dpu_image_map->height;
  }

  dpuObj->mFrame->mChannelId = leftObj->mFrame->mChannelId;
  dpuObj->mFrame->mFrameId = leftObj->mFrame->mFrameId;
  dpuObj->mFrame->mChannelIdInternal = leftObj->mFrame->mChannelIdInternal;
  dpuObj->mFrame->mHandle = leftObj->mFrame->mHandle;

    bm_image_destroy(&dpu_out);
  if (need_convert) {
    bm_image_destroy(&dpu_img[0]);
    bm_image_destroy(&dpu_img[1]);
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Dpu::doWork(int dataPipeId) {
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
    IVS_DEBUG("Got Input, port id = {0}, channel_id = {1}, frame_id = {2}",
              inputPort, objectMetadata->mFrame->mChannelId,
              objectMetadata->mFrame->mFrameId);
  }

  if (inputs[0]->mFrame->mSpData != nullptr &&
      inputs[1]->mFrame->mSpData != nullptr) {
    std::shared_ptr<common::ObjectMetadata> dpuObj =
        std::make_shared<common::ObjectMetadata>();
    dpuObj->mFrame = std::make_shared<sophon_stream::common::Frame>();
    dpu_work(inputs[0], inputs[1], dpuObj);

    IVS_INFO("Now Flag is {0}", dis_type);

    int channel_id_internal = dpuObj->mFrame->mChannelIdInternal;
    int outDataPipeId =
        getSinkElementFlag()
            ? 0
            : (channel_id_internal % getOutputConnectorCapacity(outputPort));
    common::ErrorCode errorCode = pushOutputData(
        outputPort, outDataPipeId, std::static_pointer_cast<void>(dpuObj));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(dpuObj.get()));
    }
  }

  return common::ErrorCode::SUCCESS;
}

REGISTER_WORKER("dpu", Dpu)
}  // namespace dpu
}  // namespace element
}  // namespace sophon_stream

#endif