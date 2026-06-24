//===----------------------------------------------------------------------===//
//
// Copyright (C) 2025 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_SAVE_VIDEO_H_
#define SOPHON_STREAM_ELEMENT_SAVE_VIDEO_H_

#include <atomic>
#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <vector>
#include <unordered_set>

#include "common/logger.h"
#include "common/object_metadata.h"
#include "element.h"
#include "element_factory.h"
#include "httplib.h"
#include "opencv2/opencv.hpp"

namespace sophon_stream {
namespace element {
namespace save_video {

struct ServerEndpoint {
  std::string scheme;  // http or https
  std::string host;
  int port;
  std::string path;
};

// 每通道的录像状态
struct ChannelState {
  // 录制控制
  bool recording = false;
  std::chrono::steady_clock::time_point record_end_tp;
  std::unique_ptr<cv::VideoWriter> writer;  // BM OpenCV 增强版
  int fps = 25;
  int width = 0;
  int height = 0;

  // 当前录像段共享视频路径
  std::string pending_video_path;
  // 新增：连续命中计数
  std::unordered_map<int,int> per_class_consecutive_frames; // 按类别统计

  // 多事件聚合：一个录像段内允许多个不同类别事件，共享同一视频
  struct PendingEvent {
    int class_id;               // 事件对应类别
    int type;                   // 解析得到的上报 type
    std::string img_path;       // 快照路径
    std::string datetime_str;   // 上报时间字符串（YYYY-MM-DD HH:MM:SS）
  };
  std::vector<PendingEvent> pending_events;          // 本段累计的事件
  std::unordered_set<int>   suppressed_classes;       // 已经触发过的类别（本段内不再重复）
};

class SaveVideo : public ::sophon_stream::framework::Element {
 public:
  SaveVideo();
  ~SaveVideo() override;

  common::ErrorCode initInternal(const std::string& json) override;
  common::ErrorCode doWork(int dataPipeId) override;

  // 配置字段
  static constexpr const char* CONFIG_SERVER_URL = "server_url";         // 完整URL
  static constexpr const char* CONFIG_SAVE_DIR = "save_dir";             // 根目录
  static constexpr const char* CONFIG_BASE_FILE_URL = "base_file_url";   // 用于拼接返回URL，可为空
  static constexpr const char* CONFIG_RECORD_SECONDS = "record_seconds"; // 默认10
  static constexpr const char* CONFIG_TRIGGER_CLASSES = "trigger_classes";   // 为空则任意目标

  // 存储清理
  static constexpr const char* CONFIG_RETENTION_DAYS = "retention_days";                // 超过天数删除，0表示不按天清理
  static constexpr const char* CONFIG_RETENTION_MAX_GB = "retention_max_gb";            // 超过总容量(GB)删除最旧，0表示不按容量清理
  static constexpr const char* CONFIG_CLEANUP_INTERVAL_SECONDS = "cleanup_interval_seconds"; // 清理轮询间隔，默认300秒

  // 上报字段（固定协议）
  static constexpr const char* CONFIG_DEVICE_ID = "deviceId";
  static constexpr const char* CONFIG_DEVICE_IP = "deviceIp";
  static constexpr const char* CONFIG_SAFETY_ID = "safetyId";
  static constexpr const char* CONFIG_SAFETY_NAME = "safetyName";
  static constexpr const char* CONFIG_WARNING = "warning";
  static constexpr const char* CONFIG_TYPE = "type";  // int | object(map)
  static constexpr const char* CONFIG_VIDEO_URL_FIELD = "video_url_field";  // "safetyUrl" or "brakeUrl"

 private:
  // 解析URL到端点
  static std::optional<ServerEndpoint> parseUrl(const std::string& url);
  // 写图像
  static bool saveSnapshot(const common::Frame& frame, const std::string& filepath);
  // 打开录像
  bool startRecording(int channel, const common::Frame& frame, const std::string& filepath);
  // 追加一帧
  void appendFrame(int channel, const common::Frame& frame);
  // 结束录像
  void stopRecording(int channel);
  // 构造可访问URL
  std::string makeUrl(const std::string& file_abs_path) const;
  // 上报
  void postAlarm(int channel, const std::string& imgPath, const std::string& videoPath, const std::string& datetime_str, int resolved_type);

  // 配置
  std::string server_url_;
  std::optional<ServerEndpoint> endpoint_;
  std::string save_dir_;
  std::string base_file_url_;
  int record_seconds_ = 10;
  std::unordered_set<int> trigger_classes_;

  // 存储清理配置
  int retention_days_ = 0;
  double retention_max_gb_ = 0.0;
  int cleanup_interval_seconds_ = 300;

  // 上报固定字段
  std::string device_id_ = "";
  std::string device_ip_ = "";
  std::string safety_id_ = "";
  std::string safety_name_ = "";
  std::string warning_ = "";
  // type 解析：
  // - 若为固定int，使用 fixed_type_.
  // - 若为object映射，填入 type_map_，按 class_id -> type 解析。
  // - 若未配置或为空，则使用检测到的 class_id 作为 type。
  std::optional<int> fixed_type_{};
  std::unordered_map<int,int> type_map_{};
  bool use_class_id_type_ = false; // 显式配置使用检测到的 class_id 作为上报 type
  std::string video_url_field_ = "safetyUrl";
  // 触发连续帧阈值配置
  static constexpr const char* CONFIG_MIN_TRIGGER_FRAMES = "min_trigger_frames"; // int | object
  int global_min_trigger_frames_ = 1; // 默认 1（修改：原默认5）
  std::unordered_map<int,int> per_class_min_trigger_frames_; // class_id -> threshold

  // 统一解析本次事件的 type
  int resolveType(const std::vector<std::shared_ptr<common::DetectedObjectMetadata>>& dets) const;

  // 状态
  std::mutex mtx_;
  std::unordered_map<int, ChannelState> channels_;

  // 清理线程
  std::atomic<bool> cleanup_running_{false};
  std::thread cleanup_thread_;
  void cleanupLoop();
};

}  // namespace save_video
}  // namespace element
}  // namespace sophon_stream

#endif  // SOPHON_STREAM_ELEMENT_SAVE_VIDEO_H_
