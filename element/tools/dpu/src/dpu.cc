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
  grp_->dcc_dir_en = BMCV_DPU_DCC_DIR_A13;
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
  grp_->disp_range_en = BMCV_DPU_DISP_RANGE_128;
  grp_->disp_start_pos = 30;
  grp_->dcc_dir_en = BMCV_DPU_DCC_DIR_A13;
  grp_->dpu_census_shift = 1;
  grp_->dpu_rshift1 = 3;
  grp_->dpu_rshift2 = 2;
  grp_->dpu_ca_p1 = 1800;
  grp_->dpu_ca_p2 = 14400;
  grp_->dpu_uniq_ratio = 10;
  grp_->dpu_disp_shift = 4;
  fgs_grp->depth_unit_en = BMCV_DPU_DEPTH_UNIT_MM;
  fgs_grp->fgs_max_count = 19;
  fgs_grp->fgs_max_t = 3;
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
    resp.rgcMap["DpuMode"] = rDpuModeMap[dpu_online_mode];
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
    dpu_online_mode = rsc.DpuMode;
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
    resp.rgcMap["DpuMode"] = rDpuModeMap[dpu_online_mode];
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
  // listener->setHandler(dispTypeStr.c_str(),
  //                      sophon_stream::framework::RequestType::PUT,
  //                      std::bind(&Dpu::setDispType, this,
  //                      std::placeholders::_1,
  //                                std::placeholders::_2));
  return;
}

common::ErrorCode Dpu::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  auto configure = nlohmann::json::parse(json, nullptr, false);
  if (!configure.is_object()) {
    errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
  }
  mFpsProfiler.config("fps_dpu:", 100);

  dpu_online_mode = DPU_ONLINE_MUX0;
  dpu_sgbm_mode = DPU_SGBM_MUX2;
  dpu_fgs_mode = DPU_FGS_MUX0;

  // set_sgbm_default_param(&dpu_sgbm_attr);
  set_online_default_param(&dpu_sgbm_attr, &dpu_fgs_attr);

  dpu_sgbm_attr.disp_start_pos =
      configure.find(CONFIG_INTERNAL_DISP_START_POS_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_census_shift =
      configure.find(CONFIG_INTERNAL_DPU_CENSUS_SHIFT_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_rshift1 =
      configure.find(CONFIG_INTERNAL_DPU_RSHIFT1_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_rshift2 =
      configure.find(CONFIG_INTERNAL_DPU_RSHIFT2_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_ca_p1 =
      configure.find(CONFIG_INTERNAL_DPU_CA_P1_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_ca_p2 =
      configure.find(CONFIG_INTERNAL_DPU_CA_P2_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_uniq_ratio =
      configure.find(CONFIG_INTERNAL_DPU_UNIQ_RATIO_FIELD)->get<int>();
  dpu_sgbm_attr.dpu_disp_shift =
      configure.find(CONFIG_INTERNAL_DPU_DISP_SHIFT_FIELD)->get<int>();

  auto dpu_type_str =
      configure.find(CONFIG_INTERNAL_DPU_TYPE_FILED)->get<std::string>();
  dpu_type = dpu_type_map[dpu_type_str];
  auto dpu_mode_str =
      configure.find(CONFIG_INTERNAL_DPU_MODE_FILED)->get<std::string>();

  if (dpu_type == DPU_ONLINE) {
    dpu_online_mode = online_mode_map[dpu_mode_str];
    dpu_fgs_attr.fgs_max_count =
        configure.find(CONFIG_INTERNAL_FGS_MAX_COUNT_FIELD)->get<int>();
    dpu_fgs_attr.fgs_max_t =
        configure.find(CONFIG_INTERNAL_FGS_MAX_T_FIELD)->get<int>();
    dpu_fgs_attr.fxbase_line =
        configure.find(CONFIG_INTERNAL_FXBASE_LINE_FIELD)->get<int>();
  } else if (dpu_type == DPU_SGBM) {
    dpu_sgbm_mode = sgbm_mode_map[dpu_mode_str];
  } else if (dpu_type == DPU_FGS) {
    dpu_fgs_mode = fgs_mode_map[dpu_mode_str];
    dpu_fgs_attr.fgs_max_count =
        configure.find(CONFIG_INTERNAL_FGS_MAX_COUNT_FIELD)->get<int>();
    dpu_fgs_attr.fgs_max_t =
        configure.find(CONFIG_INTERNAL_FGS_MAX_T_FIELD)->get<int>();
    dpu_fgs_attr.fxbase_line =
        configure.find(CONFIG_INTERNAL_FXBASE_LINE_FIELD)->get<int>();
  }

  return common::ErrorCode::SUCCESS;
}

common::ErrorCode Dpu::dpu_work(
    std::shared_ptr<common::ObjectMetadata> leftObj,
    std::shared_ptr<common::ObjectMetadata> rightObj,
    std::shared_ptr<common::ObjectMetadata> dpuObj) {
  bm_status_t ret;
  // dpu
  std::shared_ptr<bm_image> dpu_out = nullptr;
  // dpu输出结果
  dpu_out.reset(new bm_image, [](bm_image* p) {
    bm_image_destroy(*p);
    delete p;
    p = nullptr;
  });

  bm_image_create(leftObj->mFrame->mHandle, leftObj->mFrame->mSpDataDwa->height,
                  leftObj->mFrame->mSpDataDwa->width, dpu_fmt,
                  DATA_TYPE_EXT_1N_BYTE, dpu_out.get());
  bm_image_alloc_dev_mem(*dpu_out, 1);

  if (dpu_type == DPU_ONLINE) {
    bmcv_dpu_online_disp(leftObj->mFrame->mHandle,
                         leftObj->mFrame->mSpDataDwa.get(),
                         rightObj->mFrame->mSpDataDwa.get(), dpu_out.get(),
                         &dpu_sgbm_attr, &dpu_fgs_attr, dpu_online_mode);

  } else if (dpu_type == DPU_SGBM) {
    // dpu_sgbm_mode = DPU_SGBM_MUX2;
    bmcv_dpu_sgbm_disp(leftObj->mFrame->mHandle,
                       leftObj->mFrame->mSpDataDwa.get(),
                       rightObj->mFrame->mSpDataDwa.get(), dpu_out.get(),
                       &dpu_sgbm_attr, dpu_sgbm_mode);

  } else if (dpu_type == DPU_FGS) {
    bm_image sgbm_out;

    bm_image_create(leftObj->mFrame->mHandle,
                    leftObj->mFrame->mSpDataDwa->height,
                    leftObj->mFrame->mSpDataDwa->width, dpu_fmt,
                    DATA_TYPE_EXT_1N_BYTE, &sgbm_out, NULL);
    bm_image_alloc_dev_mem(sgbm_out, 1);

    // dpu_sgbm_mode = DPU_SGBM_MUX2;
    bmcv_dpu_sgbm_disp(leftObj->mFrame->mHandle,
                       leftObj->mFrame->mSpDataDwa.get(),
                       rightObj->mFrame->mSpDataDwa.get(), &sgbm_out,
                       &dpu_sgbm_attr, dpu_sgbm_mode);
    bmcv_dpu_fgs_disp(leftObj->mFrame->mHandle,
                      leftObj->mFrame->mSpDataDwa.get(), &sgbm_out,
                      dpu_out.get(), &dpu_fgs_attr, dpu_fgs_mode);
    bm_image_destroy(sgbm_out);
  }

  dpuObj->mFrame->mSpDataDpu = dpu_out;
  dpuObj->mFrame->mWidth = dpuObj->mFrame->mSpDataDpu->width;
  dpuObj->mFrame->mHeight = dpuObj->mFrame->mSpDataDpu->height;

  // dpuObj->mFrame->mSpData = leftObj->mFrame->mSpData;
  dpuObj->mFrame->mSpData = dpu_out;
  dpuObj->mFrame->mSpDataDwa = leftObj->mFrame->mSpDataDwa;

  dpuObj->mFrame->mChannelId = leftObj->mFrame->mChannelId;
  dpuObj->mFrame->mFrameId = leftObj->mFrame->mFrameId;
  dpuObj->mFrame->mChannelIdInternal = leftObj->mFrame->mChannelIdInternal;
  dpuObj->mFrame->mHandle = leftObj->mFrame->mHandle;
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
      inputs[1]->mFrame->mSpData != nullptr &&
      inputs[0]->mFrame->mFrameId == inputs[1]->mFrame->mFrameId) {
    std::shared_ptr<common::ObjectMetadata> dpuObj =
        std::make_shared<common::ObjectMetadata>();
    dpuObj->mFrame = std::make_shared<sophon_stream::common::Frame>();

    dpu_work(inputs[0], inputs[1], dpuObj);

    mFpsProfiler.add(1);

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