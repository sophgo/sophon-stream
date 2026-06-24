//===----------------------------------------------------------------------===//
//
// Copyright (C) 2025 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "save_video.h"

#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>

#include "common/common_defs.h"
#include <unordered_set>
#include "common/logger.h"
#include "element_factory.h"

namespace fs = std::filesystem;

namespace sophon_stream {
namespace element {
namespace save_video {

using namespace std::chrono;

SaveVideo::SaveVideo() {}
SaveVideo::~SaveVideo() {
  cleanup_running_ = false;
  if (cleanup_thread_.joinable()) cleanup_thread_.join();
}

static std::string now_datetime_str() {
  auto now = system_clock::now();
  std::time_t t = system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  char buf[32];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
  return std::string(buf);
}

std::optional<ServerEndpoint> SaveVideo::parseUrl(const std::string& url) {
  // scheme://host:port/path
  std::regex re(R"((http|https)://([^/:]+)(?::(\d+))?(/.*))");
  std::smatch m;
  if (!std::regex_match(url, m, re)) return std::nullopt;
  ServerEndpoint ep;
  ep.scheme = m[1].str();
  ep.host = m[2].str();
  ep.port = m[3].matched ? std::stoi(m[3].str()) : (ep.scheme == "https" ? 443 : 80);
  ep.path = m[4].str();
  return ep;
}

bool SaveVideo::saveSnapshot(const common::Frame& frame, const std::string& filepath) {
  try {
    fs::create_directories(fs::path(filepath).parent_path());
    cv::Mat img;
    // 优先使用已绘制 OSD 的图像
    if (frame.mSpDataOsd) {
      cv::bmcv::toMAT(frame.mSpDataOsd.get(), img, true);
    } else if (frame.mSpData) {
      cv::bmcv::toMAT(frame.mSpData.get(), img, true);
    } else if (!frame.mMat.empty()) {
      img = frame.mMat;
    } else {
      return false;
    }
    return cv::imwrite(filepath, img);
  } catch (const std::exception& e) {
    IVS_ERROR("saveSnapshot error: {0}", e.what());
    return false;
  }
}

bool SaveVideo::startRecording(int channel, const common::Frame& frame, const std::string& filepath) {
  auto& st = channels_[channel];
  try {
    fs::create_directories(fs::path(filepath).parent_path());
    int fps = st.fps > 0 ? st.fps : (frame.mFrameRate.mDenominator != 0 ? std::max(1, frame.mFrameRate.mNumber / std::max(1, frame.mFrameRate.mDenominator)) : 25);
    int w = frame.mWidth;
    int h = frame.mHeight;
    if (w <= 0 || h <= 0) return false;
    st.width = w;
    st.height = h;
    st.fps = fps;
  // （预录功能已移除，不再计算预录缓存大小）

    st.writer = std::make_unique<cv::VideoWriter>();
  // MP4 容器更推荐使用 'avc1' FourCC，避免 OpenCV/FFmpeg 警告并保持 H.264 编码
  int fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
    bool ok = st.writer->open(filepath, fourcc, fps, cv::Size(w, h));
    if (!ok) {
      IVS_ERROR("open VideoWriter failed: {0}", filepath);
      st.writer.reset();
      return false;
    }
    st.recording = true;
    st.record_end_tp = steady_clock::now() + seconds(record_seconds_);
    return true;
  } catch (const std::exception& e) {
    IVS_ERROR("startRecording error: {0}", e.what());
    return false;
  }
}

void SaveVideo::appendFrame(int channel, const common::Frame& frame) {
  auto it = channels_.find(channel);
  if (it == channels_.end()) return;
  auto& st = it->second;
  if (!st.recording || !st.writer) return;
  cv::Mat img;
  // 优先写入 OSD 后的图像
  if (frame.mSpDataOsd) {
    cv::bmcv::toMAT(frame.mSpDataOsd.get(), img, true);
  } else if (frame.mSpData) {
    cv::bmcv::toMAT(frame.mSpData.get(), img, true);
  } else if (!frame.mMat.empty()) {
    img = frame.mMat;
  } else {
    return;
  }
  if (img.cols != st.width || img.rows != st.height) {
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(st.width, st.height));
    st.writer->write(resized);
  } else {
    st.writer->write(img);
  }
}

void SaveVideo::stopRecording(int channel) {
  auto it = channels_.find(channel);
  if (it == channels_.end()) return;
  auto& st = it->second;
  if (st.writer) {
    st.writer->release();
  }
  st.writer.reset();
  st.recording = false;
}

std::string SaveVideo::makeUrl(const std::string& file_abs_path) const {
  if (base_file_url_.empty()) return file_abs_path;  // 直接返回本地路径
  // 将 save_dir_ 作为根去掉前缀
  std::string rel = file_abs_path;
  if (!save_dir_.empty()) {
    try {
      fs::path relp = fs::relative(file_abs_path, save_dir_);
      rel = relp.generic_string();
    } catch (...) {
    }
  }
  if (base_file_url_.back() == '/') return base_file_url_ + rel;
  return base_file_url_ + "/" + rel;
}

void SaveVideo::postAlarm(int channel, const std::string& imgPath, const std::string& videoPath, const std::string& datetime_str, int resolved_type) {
  if (!endpoint_.has_value()) return;
  const auto& ep = endpoint_.value();
  try {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    httplib::Client cli(ep.scheme + "://" + ep.host + ":" + std::to_string(ep.port));
#else
    httplib::Client cli(ep.host, ep.port);
#endif
    nlohmann::json j;
    j["deviceId"] = device_id_;
    j["deviceIp"] = device_ip_;
    j["safetyId"] = safety_id_;
    j["safetyName"] = safety_name_;
    j["warning"] = warning_;
    j["type"] = resolved_type;
    if (video_url_field_ == "safetyUrl") {
      j["safetyUrl"] = makeUrl(videoPath);
      j["brakeUrl"] = "";
    } else {
      j["brakeUrl"] = makeUrl(videoPath);
      j["safetyUrl"] = "";
    }
    j["datatime"] = datetime_str;
    j["imgUrl"] = makeUrl(imgPath);
    auto res = cli.Post(ep.path.c_str(), j.dump(), "application/json");
    if (!res) {
      IVS_ERROR("save_video http post failed: no response");
    } else if (res->status != 200) {
      IVS_ERROR("save_video http status: {0}", res->status);
    } else {
      IVS_INFO("save_video http ok: status=200, len={0}", res->body.size());
    }
  } catch (const std::exception& e) {
    IVS_ERROR("save_video http exception: {0}", e.what());
  }
}

common::ErrorCode SaveVideo::initInternal(const std::string& json) {
  auto cfg = nlohmann::json::parse(json, nullptr, false);
  if (!cfg.is_object()) return common::ErrorCode::PARSE_CONFIGURE_FAIL;

  // 处理嵌套的 "configure" 对象
  if (cfg.contains("configure") && cfg["configure"].is_object()) {
    cfg = cfg["configure"];  // 直接使用内层对象
    IVS_INFO("save_video: using nested 'configure' object");
  }

  // 基础配置
  if (cfg.contains(CONFIG_SERVER_URL)) server_url_ = cfg[CONFIG_SERVER_URL].get<std::string>();
  endpoint_ = parseUrl(server_url_);
  if (!cfg.contains(CONFIG_SAVE_DIR)) return common::ErrorCode::PARSE_CONFIGURE_FAIL;
  save_dir_ = cfg[CONFIG_SAVE_DIR].get<std::string>();
  if (cfg.contains(CONFIG_BASE_FILE_URL)) base_file_url_ = cfg[CONFIG_BASE_FILE_URL].get<std::string>();
  if (cfg.contains(CONFIG_RECORD_SECONDS)) record_seconds_ = std::max(1, cfg[CONFIG_RECORD_SECONDS].get<int>());

  if (cfg.contains(CONFIG_TRIGGER_CLASSES)) {
    trigger_classes_.clear();
    const auto& tc = cfg[CONFIG_TRIGGER_CLASSES];
    if (tc.is_array()) {
      for (const auto& item : tc) {
        if (item.is_number_integer()) {
          trigger_classes_.insert(item.get<int>());
        }
      }
    }
    if (!trigger_classes_.empty()) {
      std::stringstream ss;
      ss << "save_video: trigger_classes (by id) loaded {";
      bool first = true;
      for (int cid : trigger_classes_) {
        if (!first) ss << ", ";
        first = false;
        ss << cid;
      }
      ss << "}";
      IVS_INFO("{0}", ss.str());
    }
  }
  // 连续帧阈值解析（默认值已在头文件成员初始化中为 1，这里仅解析配置覆盖）
  per_class_min_trigger_frames_.clear();
  if (cfg.contains(CONFIG_MIN_TRIGGER_FRAMES)) {
    const auto &mtf = cfg[CONFIG_MIN_TRIGGER_FRAMES];
    if (mtf.is_number_integer()) {
      global_min_trigger_frames_ = std::max(1, mtf.get<int>());
      IVS_INFO("save_video: global min_trigger_frames = {0}", global_min_trigger_frames_);
    } else if (mtf.is_object()) {
      for (auto it = mtf.begin(); it != mtf.end(); ++it) {
        try {
          int cid = std::stoi(it.key());
          if (it.value().is_number_integer()) {
            int thr = std::max(1, it.value().get<int>());
            per_class_min_trigger_frames_[cid] = thr;
          }
        } catch (...) {}
      }
      if (!per_class_min_trigger_frames_.empty()) {
        std::stringstream ss; ss << "save_video: per-class min_trigger_frames {"; bool first=true; for (auto &kv: per_class_min_trigger_frames_) { if(!first) ss << ", "; first=false; ss<<kv.first<<"->"<<kv.second; } ss << "}, default=" << global_min_trigger_frames_; IVS_INFO("{0}", ss.str());
      }
    }
  }

  // 存储清理
  if (cfg.contains(CONFIG_RETENTION_DAYS)) retention_days_ = std::max(0, cfg[CONFIG_RETENTION_DAYS].get<int>());
  if (cfg.contains(CONFIG_RETENTION_MAX_GB)) retention_max_gb_ = std::max(0.0, cfg[CONFIG_RETENTION_MAX_GB].get<double>());
  if (cfg.contains(CONFIG_CLEANUP_INTERVAL_SECONDS)) cleanup_interval_seconds_ = std::max(30, cfg[CONFIG_CLEANUP_INTERVAL_SECONDS].get<int>());

  // 协议字段
  if (cfg.contains(CONFIG_DEVICE_ID)) device_id_ = cfg[CONFIG_DEVICE_ID].get<std::string>();
  if (cfg.contains(CONFIG_DEVICE_IP)) device_ip_ = cfg[CONFIG_DEVICE_IP].get<std::string>();
  if (cfg.contains(CONFIG_SAFETY_ID)) safety_id_ = cfg[CONFIG_SAFETY_ID].get<std::string>();
  if (cfg.contains(CONFIG_SAFETY_NAME)) safety_name_ = cfg[CONFIG_SAFETY_NAME].get<std::string>();
  if (cfg.contains(CONFIG_WARNING)) warning_ = cfg[CONFIG_WARNING].get<std::string>();
  if (cfg.contains(CONFIG_TYPE)) {
    const auto& tv = cfg[CONFIG_TYPE];
    if (tv.is_number_integer()) {
      fixed_type_ = tv.get<int>();
      use_class_id_type_ = false;
      type_map_.clear();
      IVS_INFO("save_video: configured fixed_type = {0}", *fixed_type_);
    } else if (tv.is_object()) {
      // 解析 class_id -> type 的映射
      fixed_type_.reset();
      use_class_id_type_ = false;
      type_map_.clear();
      for (auto it = tv.begin(); it != tv.end(); ++it) {
        try {
          int class_id = std::stoi(it.key());
          if (it.value().is_number_integer()) {
            type_map_[class_id] = it.value().get<int>();
          }
        } catch (...) {
          // 非法键名忽略
        }
      }
      std::stringstream ss;
      ss << "save_video: type_map loaded {";
      bool first = true;
      for (auto &kv : type_map_) {
        if (!first) ss << ", ";
        first = false;
        ss << kv.first << "->" << kv.second;
      }
      ss << "}";
      IVS_INFO("{0}", ss.str());
    } else if (tv.is_string()) {
      std::string s = tv.get<std::string>();
      if (s == "class_id" || s == "classid" || s == "label" ) {
        // 显式指明使用检测到的第一个目标 class_id 作为 type
        fixed_type_.reset();
        type_map_.clear();
        use_class_id_type_ = true;
        IVS_INFO("save_video: configured use_class_id_type = true");
      }
    } else {
      IVS_WARN("save_video: unsupported type field format");
    }
  } else {
    IVS_INFO("save_video: no type field found, will use default class_id");
  }
  if (cfg.contains(CONFIG_VIDEO_URL_FIELD)) {
    video_url_field_ = cfg[CONFIG_VIDEO_URL_FIELD].get<std::string>();
    if (video_url_field_ != "safetyUrl" && video_url_field_ != "brakeUrl") {
      video_url_field_ = "safetyUrl";
    }
  }

  try { fs::create_directories(save_dir_); } catch (...) {}
  // 启动清理线程（若配置了任一条件）
  if ((retention_days_ > 0 || retention_max_gb_ > 0.0) && !save_dir_.empty()) {
    cleanup_running_ = true;
    cleanup_thread_ = std::thread(&SaveVideo::cleanupLoop, this);
  }
  return common::ErrorCode::SUCCESS;
}

common::ErrorCode SaveVideo::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  // sink 元素也需要向引擎上报数据：按照约定，sink 使用 outputPort=0
  int outputPort = getSinkElementFlag() ? 0 : getOutputPorts()[0];

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto obj = std::static_pointer_cast<common::ObjectMetadata>(data);
  if (!obj->mFrame || obj->mFrame->mEndOfStream) {
    // 标记 EOF 帧为过滤，避免被上层统计为有效帧
    if (obj && obj->mFrame && obj->mFrame->mEndOfStream) {
      obj->mFilter = true;
    }
    // 透传（包括 sink）：sink 固定用 outDataPipeId=0
    int outDataPipeId = getSinkElementFlag() ? 0 : 0;
    pushOutputData(outputPort, outDataPipeId, obj);
    return common::ErrorCode::SUCCESS;
  }

  int ch = obj->mFrame->mChannelIdInternal;
  auto& st = channels_[ch];

  // 这是有效帧，确保不被统计逻辑过滤
  obj->mFilter = false;

  // （预录功能已移除，不再维护回溯帧缓存）

  // 条件：检测到目标（若 trigger_classes_ 非空，则类别需匹配）
  auto hasTarget = [&]() {
    if (!trigger_classes_.empty()) {
      for (auto& det : obj->mDetectedObjectMetadatas) {
        if (det && trigger_classes_.count(det->mClassify)) {
          return true;
        }
      }
      return false;
    } else {
      return !obj->mDetectedObjectMetadatas.empty();
    }
  }();

  auto now_tp = steady_clock::now();
  // 冷却机制已移除

  // 连续帧统计（仅按类别）：记录“本帧首次达到阈值且尚未被抑制”的类别
  std::vector<int> newly_reached_classes;

  if (hasTarget) {
    // 按类别：同一帧同一个 class_id 只计一次
    std::unordered_set<int> frame_class_ids;
    frame_class_ids.reserve(obj->mDetectedObjectMetadatas.size());
    for (auto &det : obj->mDetectedObjectMetadatas) {
      if (!det) continue;
      int cid = det->mClassify;
      if (cid < 0) continue;
      // 修正：只有在 trigger_classes_ 为空或 cid 在其中时，才处理该类别
      if (trigger_classes_.empty() || trigger_classes_.count(cid)) {
        frame_class_ids.insert(cid);
      }
    }
    for (int cid : frame_class_ids) {
      auto &cnt = st.per_class_consecutive_frames[cid];
      int prev = cnt;
      cnt++;
      int need = global_min_trigger_frames_;
      auto itThr = per_class_min_trigger_frames_.find(cid);
      if (itThr != per_class_min_trigger_frames_.end()) need = itThr->second;
      // 仅在首次达到阈值且该类别尚未被段内抑制时记录
      if (cnt >= need && prev < need && st.suppressed_classes.count(cid)==0) {
        newly_reached_classes.push_back(cid);
      }
    }
      // 严格连续语义：本帧缺失的已存在类别计数归零（下一次再出现重新累计）
      if (!st.per_class_consecutive_frames.empty()) {
        std::vector<int> cids_to_reset;
        for (auto &kv : st.per_class_consecutive_frames) {
          int cid_existing = kv.first;
          // 修正：只处理 trigger_classes_ 相关的计数器
          if (trigger_classes_.empty() || trigger_classes_.count(cid_existing)) {
            if (frame_class_ids.find(cid_existing) == frame_class_ids.end()) {
              cids_to_reset.push_back(cid_existing);
            }
          }
        }
        for (int cid : cids_to_reset) {
          st.per_class_consecutive_frames[cid] = 0;
        }
      }
  } else {
    // 无目标：清空所有相关计数
    if (!trigger_classes_.empty()) {
      for (int cid : trigger_classes_) {
        st.per_class_consecutive_frames[cid] = 0;
      }
    } else {
      st.per_class_consecutive_frames.clear();
    }
  }
  // 仅当有“新首次达到阈值”的类别时触发
  if (!newly_reached_classes.empty()) {
    // 触发前调试日志：打印达到阈值的类别以及当前全部计数
    {
      std::stringstream ssAll;
  ssAll << "save_video trigger per={";
      bool first=true; for (auto &kv: st.per_class_consecutive_frames) { if(!first) ssAll<<","; first=false; ssAll<<kv.first<<":"<<kv.second; }
      ssAll << "} fired={";
      for (size_t i=0;i<newly_reached_classes.size();++i){ if(i) ssAll<<","; ssAll<<newly_reached_classes[i]; }
      ssAll << "}";
      IVS_INFO("{0}", ssAll.str());
    }
    // 不再截断当前录像：若已在录制，只追加事件
    // 生成文件名：save_dir/channel_#/YYYYmmdd_HHMMSS
    auto now_sys = system_clock::now();
    std::time_t t = system_clock::to_time_t(now_sys);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
  int resolved_type = resolveType(obj->mDetectedObjectMetadatas);

  // 多类别同时触发：全部记录；首个类别用于主命名（base.jpg）
  int primary_class = newly_reached_classes.empty() ? -1 : newly_reached_classes.front();

  // 生成包含 type 的文件名
  char timebuf[32];
  std::strftime(timebuf, sizeof(timebuf), "%Y%m%d_%H%M%S", &tm);
  std::string timestr(timebuf);
  std::string base_name = timestr + "_type" + std::to_string(resolved_type);
  std::string base = (fs::path(save_dir_) / ("ch_" + std::to_string(ch)) / base_name).string();
    std::vector<int> classes_to_add; // 本次真正需要记录的类别（去除已抑制）
    for (int cid : newly_reached_classes) {
      if (cid >= 0 && st.suppressed_classes.count(cid)) continue; // 已记录
      classes_to_add.push_back(cid);
    }

    std::string vidPath;
    if (!st.recording) {
      vidPath = base + ".mp4";
      bool record_ok = startRecording(ch, *obj->mFrame, vidPath);
  st.pending_video_path = vidPath;
      st.pending_events.clear();
      st.suppressed_classes.clear();
      if (!record_ok) {
        // 录像失败：按类别直接上报（共享一张主图片）
        std::string mainImg = base + ".jpg";
        saveSnapshot(*obj->mFrame, mainImg);
        char dtbuf[32]; std::strftime(dtbuf, sizeof(dtbuf), "%Y-%m-%d %H:%M:%S", &tm);
        for (int cid : classes_to_add) {
          ChannelState::PendingEvent ev{cid, resolved_type, mainImg, dtbuf};
          postAlarm(ch, ev.img_path, "", ev.datetime_str, ev.type);
        }
  // 无录像句柄，无需后续段结束批量上报
      } else {
        // 成功开始录像：为每个类别生成快照
        {
          std::stringstream ss;
          ss << "save_video: segment start video=" << vidPath << " classes=";
          bool first = true;
          for (int cid : classes_to_add) { if(!first) ss << ","; first=false; ss << cid; }
          ss << " type=" << resolved_type;
          IVS_INFO("{0}", ss.str());
        }
        int idx = 0;
        for (int cid : classes_to_add) {
          std::string imgPath;
          if (idx == 0) {
            imgPath = base + ".jpg"; // 主文件名
          } else {
            // 统一命名以 type 结尾：时间 + _cls<cid>_type<type>.jpg
            imgPath = (fs::path(save_dir_) / ("ch_" + std::to_string(ch)) / (timestr + "_cls" + std::to_string(cid) + "_type" + std::to_string(resolved_type) + ".jpg")).string();
          }
          saveSnapshot(*obj->mFrame, imgPath);
          char dtbuf[32]; std::strftime(dtbuf, sizeof(dtbuf), "%Y-%m-%d %H:%M:%S", &tm);
          ChannelState::PendingEvent ev{cid, resolved_type, imgPath, dtbuf};
          st.pending_events.push_back(std::move(ev));
          st.suppressed_classes.insert(cid);
          ++idx;
        }
      }
    } else {
      vidPath = st.pending_video_path; // 共享视频
      int idx = 0;
      for (int cid : classes_to_add) {
        std::string imgPath;
        // 统一命名以 type 结尾：时间 + _cls<cid>_type<type>.jpg
        imgPath = (fs::path(save_dir_) / ("ch_" + std::to_string(ch)) / (timestr + "_cls" + std::to_string(cid) + "_type" + std::to_string(resolved_type) + ".jpg")).string();
        saveSnapshot(*obj->mFrame, imgPath);
        char dtbuf[32]; std::strftime(dtbuf, sizeof(dtbuf), "%Y-%m-%d %H:%M:%S", &tm);
        ChannelState::PendingEvent ev{cid, resolved_type, imgPath, dtbuf};
        st.pending_events.push_back(std::move(ev));
        st.suppressed_classes.insert(cid);
        ++idx;
      }
    }

    // 不再清零计数：保持计数防止阈值=1 类别因清零再次重复触发（抑制由 suppressed_classes 保证）
  }

  // 录像续写与结束
  if (st.recording) {
    appendFrame(ch, *obj->mFrame);
    if (now_tp >= st.record_end_tp) {
      stopRecording(ch);
      // 结束时批量上报本段的所有事件（共享视频路径）
      if (st.pending_events.empty()) {
        // 无事件：删除空段视频，避免产生“无告警视频”与后续 HTTP 数量难以对齐
        if (!st.pending_video_path.empty()) {
          std::error_code ec_rm;
          std::filesystem::remove(st.pending_video_path, ec_rm);
          if (!ec_rm) {
            IVS_INFO("save_video: segment end (no events) removed video={0}", st.pending_video_path);
          } else {
            IVS_WARN("save_video: segment end (no events) remove failed video={0}", st.pending_video_path);
          }
        }
      } else {
        // 汇总日志（在逐条上报之前打印）
        {
          std::stringstream ss;
          ss << "save_video: segment end video=" << st.pending_video_path << " events=" << st.pending_events.size() << " [";
          bool first=true;
          for (auto &ev: st.pending_events) {
            if (!first) ss << ","; first=false;
            ss << "{cls=" << ev.class_id << ",type=" << ev.type << "}";
          }
          ss << "]";
          IVS_INFO("{0}", ss.str());
        }
        for (auto &ev : st.pending_events) {
          postAlarm(ch, ev.img_path, st.pending_video_path, ev.datetime_str, ev.type);
          IVS_INFO("save_video: posted event cls={0} type={1} img={2}", ev.class_id, ev.type, ev.img_path);
        }
      }
      st.pending_events.clear();
      st.suppressed_classes.clear();
    }
  }

  // 透传下游或上报给引擎（sink 也要 push）
  int outDataPipeId = getSinkElementFlag() ? 0 : (ch % getOutputConnectorCapacity(outputPort));
  auto ec = pushOutputData(outputPort, outDataPipeId, obj);
  if (ec != common::ErrorCode::SUCCESS) {
    IVS_WARN("save_video push downstream failed");
  }

  return common::ErrorCode::SUCCESS;
}

int SaveVideo::resolveType(const std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& dets) const {
  // 移除高频调试日志：仅保留必要决策日志（映射命中/回退警告等）

  // 1. 固定值优先
  if (fixed_type_.has_value()) {
    return *fixed_type_;
  }
  // 2. 显式使用 class_id
  if (use_class_id_type_) {
    for (auto &d : dets) {
      if (d && d->mClassify >= 0) {
        int result = d->mClassify;
        return result;
      }
    }
    IVS_WARN("save_video: class_id_type mode but no valid detections, fallback to 0");
    return 0; // 无检测则回退 0
  }
  // 3. 映射
  if (!type_map_.empty()) {
    for (auto &d : dets) {
      if (!d) continue;
      int cid = d->mClassify;  // 使用 mClassify 而不是 getLabel()
      if (cid < 0) continue; // 跳过无效的 class_id
      auto it = type_map_.find(cid);
      if (it != type_map_.end()) {
        IVS_INFO("save_video: mapped class_id {0} -> type {1}", cid, it->second);
        return it->second;
      }
    }
    // 未命中映射：若有有效检测对象，用第一个的 class_id
    for (auto &d : dets) {
      if (d && d->mClassify >= 0) {
        int result = d->mClassify;
        IVS_WARN("save_video: mapping miss, fallback to class_id {0}", result);
        return result;
      }
    }
    IVS_WARN("save_video: mapping mode but no valid detections (all mClassify < 0), fallback to 0");
    return 0;
  }
  // 4. 未配置 type：使用第一个有效检测对象 class_id
  for (auto &d : dets) {
    if (d && d->mClassify >= 0) {
      int result = d->mClassify;
      IVS_INFO("save_video: default mode, using class_id {0}", result);
      return result;
    }
  }
  IVS_WARN("save_video: no detections and no fixed type, fallback to 0");
  return 0;
}

REGISTER_WORKER("save_video", SaveVideo)

}  // namespace save_video
}  // namespace element
}  // namespace sophon_stream

// ========== Cleanup loop implementation (placed in global namespace scope) ==========
namespace sophon_stream {
namespace element {
namespace save_video {

void SaveVideo::cleanupLoop() {
  using fs_entry = std::pair<fs::path, std::filesystem::file_time_type>;
  while (cleanup_running_) {
    try {
      // 1) 按天清理
      if (retention_days_ > 0) {
        auto cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * retention_days_);
        auto cutoff_fs = std::chrono::time_point_cast<std::filesystem::file_time_type::duration>(
            std::filesystem::file_time_type::clock::now() + (cutoff - std::chrono::system_clock::now()));

        for (auto& dir_entry : fs::recursive_directory_iterator(save_dir_)) {
          if (!dir_entry.is_regular_file()) continue;
          auto p = dir_entry.path();
          auto ext = p.extension().string();
          if (!(ext == ".mp4" || ext == ".jpg" || ext == ".jpeg")) continue;
          std::error_code ec;
          auto ft = fs::last_write_time(p, ec);
          if (ec) continue;
          if (ft < cutoff_fs) {
            fs::remove(p, ec);
          }
        }
      }

      // 2) 容量清理（删除最旧文件直到低于阈值）
      if (retention_max_gb_ > 0.0) {
        uintmax_t total_bytes = 0;
        std::vector<fs_entry> files;
        for (auto& dir_entry : fs::recursive_directory_iterator(save_dir_)) {
          if (!dir_entry.is_regular_file()) continue;
          auto p = dir_entry.path();
          auto ext = p.extension().string();
          if (!(ext == ".mp4" || ext == ".jpg" || ext == ".jpeg")) continue;
          std::error_code ec;
          auto sz = fs::file_size(p, ec);
          if (ec) continue;
          total_bytes += sz;
          auto ft = fs::last_write_time(p, ec);
          if (ec) continue;
          files.emplace_back(p, ft);
        }
        const double total_gb = (double)total_bytes / (1024.0 * 1024.0 * 1024.0);
        if (total_gb > retention_max_gb_) {
          std::sort(files.begin(), files.end(), [](const fs_entry& a, const fs_entry& b) {
            return a.second < b.second;  // old first
          });
          std::error_code ec;
          for (auto& f : files) {
            auto sz = fs::file_size(f.first, ec);
            fs::remove(f.first, ec);
            if (!ec && sz > 0) {
              total_bytes = (sz > total_bytes) ? 0 : (total_bytes - sz);
            }
            double gb = (double)total_bytes / (1024.0 * 1024.0 * 1024.0);
            if (gb <= retention_max_gb_) break;
          }
        }
      }
    } catch (...) {
      // ignore
    }

    for (int i = 0; i < cleanup_interval_seconds_ && cleanup_running_; ++i) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

}  // namespace save_video
}  // namespace element
}  // namespace sophon_stream
