//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#ifndef SOPHON_STREAM_ELEMENT_FILTER_H_
#define SOPHON_STREAM_ELEMENT_FILTER_H_

#include <nlohmann/json.hpp>
#include <queue>

#include "common/common_defs.h"
#include "common/logger.h"
#include "common/object_metadata.h"
#include "element_factory.h"
namespace sophon_stream {
namespace element {
namespace filter {
struct Area {
  std::vector<common::Point<int>> points;
};
class Filter_Imp {
 public:
  Filter_Imp(){};
  bool isInDirection(std::shared_ptr<common::ObjectMetadata> objectMetadata);
  bool isInPolygon(std::shared_ptr<common::ObjectMetadata> objectMetadata);
  bool isinclasses(std::shared_ptr<common::ObjectMetadata> objectMetadata);
  bool isOutsideWorkingHours(
      std::shared_ptr<common::ObjectMetadata> objectMetadata);
  bool istrack(std::shared_ptr<common::ObjectMetadata> objectMetadata,
               std::unordered_map<std::string, int>& continue_frame_num_);
  void push_class(int Class) { classes.push_back(Class); };
  void set_alert_first_frames(int alert_first_frames_) {
    alert_first_frames = alert_first_frames_;
  };
  void set_alert_frame_skip_nums(int alert_frame_skip_nums_) {
    alert_frame_skip_nums = alert_frame_skip_nums_;
  };
  void push_time(std::pair<int64_t, int64_t> time) { times.push_back(time); };
  void push_area(Area area) { areas.push_back(area); };
  void set_type(int type_) { type = type_; };
  void set_direction(int x, int y) { direction.mX = x; direction.mY = y; }
  void set_trajectory_interval(int t) { trajectory_interval = t; };

 private:
  std::vector<int> classes;
  int alert_first_frames;
  int alert_frame_skip_nums;
  std::vector<std::pair<int64_t, int64_t>> times;
  std::vector<Area> areas;
  int type;  // 筛选类型
  common::Point<int> direction; //预设方向，如不设置则不限方向筛选，如设置则只筛选轨迹方向与预设方向夹角<=90°的框
  int trajectory_interval = 5;
  int frame_count = 0;
  std::unordered_map<std::string, common::Point<int>> trajectories_cnt; // 每个trackId的轨迹数据(当前帧)。
  std::unordered_map<std::string, common::Point<int>> trajectories_pre; // 每个trackId的轨迹数据(trajectory_interval帧前)。
  bool onSegment(const common::Point<int>& p, const common::Point<int>& q,
                 const common::Point<int>& r);

  int orientation(const common::Point<int>& p, const common::Point<int>& q,
                  const common::Point<int>& r);

  bool doIntersect(const common::Point<int>& p1, const common::Point<int>& q1,
                   const common::Point<int>& p2, const common::Point<int>& q2);

  bool isPointInsidePolygon(const common::Point<int>& p,
                            const std::vector<common::Point<int>>& polygon);

  bool isRectangleInsidePolygon(std::vector<common::Point<int>>& rectangle,
                                const std::vector<common::Point<int>>& polygon);
};
class Filter : public ::sophon_stream::framework::Element {
 public:
  Filter();
  ~Filter() override;
  common::ErrorCode initInternal(const std::string& json) override;

  common::ErrorCode doWork(int dataPipeId) override;

  static constexpr const char* CONFIG_INTERNAL_RULES_FILED = "rules";
  static constexpr const char* CONFIG_INTERNAL_CHANNEL_ID_FILED = "channel_id";
  static constexpr const char* CONFIG_INTERNAL_FILTERS_FILED = "filters";

  static constexpr const char* CONFIG_INTERNAL_ALERT_FIRST_FRAME_FILED =
      "alert_first_frame";
  static constexpr const char* CONFIG_INTERNAL_ALERT_FRAME_SKIP_NUM_FILED =
      "alert_frame_skip_nums";
  static constexpr const char* CONFIG_INTERNAL_AREAS_FILED = "areas";

  static constexpr const char* CONFIG_INTERNAL_TOP_FILED = "top";
  static constexpr const char* CONFIG_INTERNAL_LEFT_FILED = "left";
  static constexpr const char* CONFIG_INTERNAL_CLASSES_FILED = "classes";
  static constexpr const char* CONFIG_INTERNAL_TIMES_FILED = "times";
  static constexpr const char* CONFIG_INTERNAL_TIME_START_FILED = "time_start";
  static constexpr const char* CONFIG_INTERNAL_TIME_END_FILED = "time_end";
  static constexpr const char* CONFIG_INTERNAL_TYPE_FILED = "type";
  static constexpr const char* CONFIG_INTERNAL_DIRECTION = "direction";
  static constexpr const char* CONFIG_INTERNAL_TRAJECTORY_INTERVAL = "trajectory_interval";

 private:
  int64_t timeToMilliseconds(
      const std::string& time);  // 把时间戳转换成一天内的时间

  std::vector<std::vector<Filter_Imp>> Filter_imps;  // 不同路的不同过滤器
  std::unordered_map<int, int>
      channel_id_indexs;  // 每一路对应的channel_id的index
  std::vector<std::unordered_map<std::string, int>>
      continue_frame_num;  // 每一路连续追踪计数，map中key代表追踪的id,value代表追踪帧数
  // std::mutex condit_mtx;
};

}  // namespace filter
}  // namespace element
}  // namespace sophon_stream

#endif