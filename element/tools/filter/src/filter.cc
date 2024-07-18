//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// SOPHON-STREAM is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "filter.h"

namespace sophon_stream {
namespace element {
namespace filter {
Filter::Filter() {}
Filter::~Filter() {}

common::ErrorCode Filter::initInternal(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);

    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    // 遍历 JSON 数组
    auto rulesIt = configure.find(CONFIG_INTERNAL_RULES_FILED);
    STREAM_CHECK((rulesIt != configure.end() && rulesIt->is_array()),
                 "rules must be array, please check your Filter element "
                 "configuration file");
    for (auto& filter_array : *rulesIt) {
      std::vector<Filter_Imp> Filter_Imp_s;
      auto channelidIt = filter_array.find(CONFIG_INTERNAL_CHANNEL_ID_FILED);
      STREAM_CHECK((channelidIt != filter_array.end() &&
                    channelidIt->is_number_integer()),
                   "channelid must be int, please check your Filter element "
                   "configuration file");
      channel_id_indexs[channelidIt->get<int>()] = Filter_imps.size();
      auto FiltersIt = filter_array.find(CONFIG_INTERNAL_FILTERS_FILED);
      STREAM_CHECK((FiltersIt != filter_array.end() && FiltersIt->is_array()),
                   "Filters must be array, please check your Filter element "
                   "configuration file");
      for (auto& filter : *FiltersIt) {
        Filter_Imp Filter_Imp_;
        Filter_Imp_.set_alert_first_frames(
            filter.value(CONFIG_INTERNAL_ALERT_FIRST_FRAME_FILED, 0));
        Filter_Imp_.set_alert_frame_skip_nums(
            filter.value(CONFIG_INTERNAL_ALERT_FRAME_SKIP_NUM_FILED, 1));

        // Parse areas
        auto areasIt = filter.find(CONFIG_INTERNAL_AREAS_FILED);

        STREAM_CHECK((areasIt != filter.end() && areasIt->is_array()),
                     "areas must be array, please check your Filter element "
                     "configuration file");
        for (auto& polygon : *areasIt) {
          STREAM_CHECK(
              (polygon.is_array()),
              "polygon must be array, please check your Filter element "
              "configuration file");
          Area area;
          for (auto& point : polygon) {
            auto topIt = point.find(CONFIG_INTERNAL_TOP_FILED);
            STREAM_CHECK((topIt != point.end() && topIt->is_number_integer()),
                         "top must be int, please check your Filter element "
                         "configuration file");
            auto leftIt = point.find(CONFIG_INTERNAL_LEFT_FILED);
            STREAM_CHECK((leftIt != point.end() && leftIt->is_number_integer()),
                         "left must be int, please check your Filter element "
                         "configuration file");
            area.points.push_back({topIt->get<int>(), leftIt->get<int>()});
          }
          Filter_Imp_.push_area(area);
        }

        // Parse classes (assuming it should be a vector of strings)
        auto classesIt = filter.find(CONFIG_INTERNAL_CLASSES_FILED);

        STREAM_CHECK((classesIt != filter.end() && classesIt->is_array()),
                     "classes must be array, please check your Filter element "
                     "configuration file");
        for (auto& cls : *classesIt) {
          STREAM_CHECK((cls.is_number_integer()),
                       "cls must be int, please check your Filter element "
                       "configuration file");
          Filter_Imp_.push_class(cls.get<int>());
        }

        // Parse times
        auto timesIt = filter.find(CONFIG_INTERNAL_TIMES_FILED);

        STREAM_CHECK((timesIt != filter.end() && timesIt->is_array()),
                     "times must be array, please check your Filter element "
                     "configuration file");
        for (auto& time_obj : *timesIt) {
          auto time_sIt = time_obj.find(CONFIG_INTERNAL_TIME_START_FILED);

          STREAM_CHECK(
              (time_sIt != time_obj.end() && time_sIt->is_string()),
              "time_start must be string, please check your Filter element "
              "configuration file");
          auto time_tIt = time_obj.find(CONFIG_INTERNAL_TIME_END_FILED);

          STREAM_CHECK(
              (time_tIt != time_obj.end() && time_tIt->is_string()),
              "time_end must be string, please check your Filter element "
              "configuration file");
          Filter_Imp_.push_time(
              {timeToMilliseconds(time_sIt->get<std::string>()),
               timeToMilliseconds(time_tIt->get<std::string>())});
        }

        // Parse type
        auto typeIt = filter.find(CONFIG_INTERNAL_TYPE_FILED);
        STREAM_CHECK((typeIt != filter.end() && typeIt->is_number_integer()),
                     "type must be int, please check your Filter element "
                     "configuration file");
        Filter_Imp_.set_type(typeIt->get<int>());
        // STREAM_CHECK((typeIt->get<int>() == 0 || typeIt->get<int>() == 1 ),
        //              "type only support 0, please check your Filter element "
        //              "configuration file");
        Filter_Imp_s.push_back(Filter_Imp_);
      }
      Filter_imps.push_back(Filter_Imp_s);
      std::unordered_map<std::string, int> continue_frame_num_;
      continue_frame_num.push_back(continue_frame_num_);
    }

  } while (false);
  return errorCode;
}

common::ErrorCode Filter::doWork(int dataPipeId) {
  std::vector<int> inputPorts = getInputPorts();
  int inputPort = inputPorts[0];
  int outputPort = 0;
  if (!getSinkElementFlag()) {
    std::vector<int> outputPorts = getOutputPorts();
    outputPort = outputPorts[0];
  }

  auto data = popInputData(inputPort, dataPipeId);
  while (!data && (getThreadStatus() == ThreadStatus::RUN)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    data = popInputData(inputPort, dataPipeId);
  }
  if (data == nullptr) return common::ErrorCode::SUCCESS;

  auto objectMetadata = std::static_pointer_cast<common::ObjectMetadata>(data);

  int channel_id_internal = objectMetadata->mFrame->mChannelIdInternal;
  objectMetadata->tag = 0;
  int outDataPipeId =
      getSinkElementFlag()
          ? 0
          : (channel_id_internal % getOutputConnectorCapacity(outputPort));
  if (objectMetadata->mFrame->mEndOfStream) {
    common::ErrorCode errorCode =
        pushOutputData(outputPort, outDataPipeId,
                       std::static_pointer_cast<void>(objectMetadata));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(objectMetadata.get()));
    }
    return errorCode;
  }
  if (channel_id_indexs.find(objectMetadata->mFrame->mChannelId) ==
      channel_id_indexs.end()) {
    IVS_WARN(
        "The current channel_id : {0:d} does not have a corresponding filter "
        "and will not be filtered. Please check the filter config",
        objectMetadata->mFrame->mChannelId);
    common::ErrorCode errorCode =
        pushOutputData(outputPort, outDataPipeId,
                       std::static_pointer_cast<void>(objectMetadata));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(objectMetadata.get()));
    }
    return errorCode;
  }

  channel_id_internal = channel_id_indexs[objectMetadata->mFrame->mChannelId];
  for (int i = 0; i < Filter_imps[channel_id_internal].size(); i++) {
    bool flag = true;
    // 时间规则

    if (objectMetadata->mDetectedObjectMetadatas.size())
      flag &= Filter_imps[channel_id_internal][i].isOutsideWorkingHours(
          objectMetadata);
    if (!flag) continue;

    flag &= Filter_imps[channel_id_internal][i].isinclasses(objectMetadata);

    if (!flag) continue;

    flag &= Filter_imps[channel_id_internal][i].isInPolygon(objectMetadata);

    if (!flag) continue;

    {
      // std::lock_guard<std::mutex> lock(condit_mtx);
      std::unordered_map<std::string, int> names;

      if (objectMetadata->mSubObjectMetadatas.size())
        for (int j = 0; j < objectMetadata->mSubObjectMetadatas.size(); j++) {
          if (objectMetadata->mSubObjectMetadatas[j]
                  ->mRecognizedObjectMetadatas.size() == 1) {
            std::string name = objectMetadata->mSubObjectMetadatas[j]
                                   ->mRecognizedObjectMetadatas[0]
                                   ->mLabelName;

            continue_frame_num[channel_id_internal][name]++;
            names[name] = continue_frame_num[channel_id_internal][name];
          }
        }
      if (objectMetadata->mTrackedObjectMetadatas.size())
        for (int j = 0; j < objectMetadata->mTrackedObjectMetadatas.size();
             j++) {
          std::string name = std::to_string(
              objectMetadata->mTrackedObjectMetadatas[j]->mTrackId);
          continue_frame_num[channel_id_internal][name]++;
          names[name] = continue_frame_num[channel_id_internal][name];
        }

      continue_frame_num[channel_id_internal].clear();
      for (auto j : names) {
        continue_frame_num[channel_id_internal][j.first] = j.second;
      }

      // continue_frame_num[channel_id_internal]=continue_frame_num_;
      flag &= Filter_imps[channel_id_internal][i].istrack(
          objectMetadata, continue_frame_num[channel_id_internal]);
      if (!flag) continue;
    }

    // 区域规则
    // ... 可以在这里添加区域规则的实现
    if (flag)
      objectMetadata->tag |=
          1
          << i;  // 二进制代表，i位是1代表满足第i个筛选器，之后在业务里根据tag判断这个数据是经过几号筛选器过滤的
  }

  if (objectMetadata->tag) {
    common::ErrorCode errorCode =
        pushOutputData(outputPort, outDataPipeId,
                       std::static_pointer_cast<void>(objectMetadata));
    if (common::ErrorCode::SUCCESS != errorCode) {
      IVS_WARN(
          "Send data fail, element id: {0:d}, output port: {1:d}, data: "
          "{2:p}",
          getId(), outputPort, static_cast<void*>(objectMetadata.get()));
    }
    return errorCode;
  }

  return common::ErrorCode::SUCCESS;
}

bool Filter_Imp::isOutsideWorkingHours(
    std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  int channel_id = objectMetadata->mFrame->mChannelIdInternal;

  for (int i = 0; i < times.size(); i++) {
    std::int64_t timestamp =
        objectMetadata->mFrame->mTimestamp % ((1000 * 60 * 60 * 24));
    int channel_id = objectMetadata->mFrame->mChannelIdInternal;
    std::int64_t timestamp_start = times[i].first;
    std::int64_t timestamp_end = times[i].second;
    if (timestamp <= timestamp_end && timestamp >= timestamp_start) return true;
  }
  return false;
}
bool Filter_Imp::isinclasses(
    std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  int channel_id = objectMetadata->mFrame->mChannelIdInternal;
  bool flag_tot = false;
  std::vector<std::shared_ptr<common::DetectedObjectMetadata>>
      new_mDetectedObjectMetadatas;
  std::vector<std::shared_ptr<common::ObjectMetadata>> new_mSubObjectMetadatas;
  std::vector<std::shared_ptr<common::TrackedObjectMetadata>>
      new_mTrackedObjectMetadatas;

  for (int j = 0; j < objectMetadata->mDetectedObjectMetadatas.size(); j++) {
    bool flag = false;

    for (int i = 0; i < classes.size(); i++) {
      int name = objectMetadata->mDetectedObjectMetadatas[j]->mClassify;

      flag |= name == classes[i];
      if (flag) break;
    }
    if (flag) {
      new_mDetectedObjectMetadatas.push_back(
          objectMetadata->mDetectedObjectMetadatas[j]);
      if (type == 0) {
        new_mSubObjectMetadatas.push_back(
            objectMetadata->mSubObjectMetadatas[j]);

      } else if (type == 1) {
        new_mTrackedObjectMetadatas.push_back(
            objectMetadata->mTrackedObjectMetadatas[j]);
      }
    }

    flag_tot |= flag;
  }

  if (flag_tot) {
    objectMetadata->mDetectedObjectMetadatas.clear();
    objectMetadata->mDetectedObjectMetadatas = new_mDetectedObjectMetadatas;
    if (type == 0) {
      objectMetadata->mSubObjectMetadatas.clear();
      objectMetadata->mSubObjectMetadatas = new_mSubObjectMetadatas;
    } else if (type == 1) {
      objectMetadata->mTrackedObjectMetadatas.clear();
      objectMetadata->mTrackedObjectMetadatas = new_mTrackedObjectMetadatas;
    }
  }

  return flag_tot;
}

bool Filter_Imp::isInPolygon(
    std::shared_ptr<common::ObjectMetadata> objectMetadata) {
  int channel_id = objectMetadata->mFrame->mChannelIdInternal;
  bool flag_tot = false;
  std::vector<std::shared_ptr<common::DetectedObjectMetadata>>
      new_mDetectedObjectMetadatas;
  std::vector<std::shared_ptr<common::ObjectMetadata>> new_mSubObjectMetadatas;
  std::vector<std::shared_ptr<common::TrackedObjectMetadata>>
      new_mTrackedObjectMetadatas;

  for (int j = 0; j < objectMetadata->mDetectedObjectMetadatas.size(); j++) {
    bool flag = false;
    int top = objectMetadata->mDetectedObjectMetadatas[j]->mBox.top();
    int bottom = objectMetadata->mDetectedObjectMetadatas[j]->mBox.bottom();
    int left = objectMetadata->mDetectedObjectMetadatas[j]->mBox.left();
    int right = objectMetadata->mDetectedObjectMetadatas[j]->mBox.right();
    std::vector<common::Point<int>> rectangle = {
        common::Point<int>(top, left), common::Point<int>(top, right),
        common::Point<int>(bottom, right), common::Point<int>(bottom, left)};

    for (int i = 0; i < areas.size(); ++i) {
      flag |= isRectangleInsidePolygon(rectangle, areas[i].points);
      if (flag) {
        objectMetadata->areas.push_back(areas[i].points);
        break;
      }
    }
    if (flag) {
      new_mDetectedObjectMetadatas.push_back(
          objectMetadata->mDetectedObjectMetadatas[j]);
      if (type == 0) {
        new_mSubObjectMetadatas.push_back(
            objectMetadata->mSubObjectMetadatas[j]);

      } else if (type == 1) {
        new_mTrackedObjectMetadatas.push_back(
            objectMetadata->mTrackedObjectMetadatas[j]);
      }
    }

    flag_tot |= flag;
  }
  if (flag_tot) {
    objectMetadata->mDetectedObjectMetadatas.clear();
    objectMetadata->mDetectedObjectMetadatas = new_mDetectedObjectMetadatas;
    if (type == 0) {
      objectMetadata->mSubObjectMetadatas.clear();
      objectMetadata->mSubObjectMetadatas = new_mSubObjectMetadatas;
    } else if (type == 1) {
      objectMetadata->mTrackedObjectMetadatas.clear();
      objectMetadata->mTrackedObjectMetadatas = new_mTrackedObjectMetadatas;
    }
  }

  return flag_tot;
}
bool Filter_Imp::istrack(
    std::shared_ptr<common::ObjectMetadata> objectMetadata,
    std::unordered_map<std::string, int>& continue_frame_num_) {
  if (type != 1 && type != 0) return true;
  int channel_id = objectMetadata->mFrame->mChannelIdInternal;
  std::unordered_map<std::string, int> up_list;
  for (auto i : continue_frame_num_) {
    if (i.second > alert_first_frames) {
      if (((i.second - alert_first_frames - 1) % alert_frame_skip_nums) == 0) {
        up_list[i.first] = 1;
      }
    }
  }
  if (up_list.size() == 0) return false;
  std::vector<std::shared_ptr<common::DetectedObjectMetadata>>
      new_mDetectedObjectMetadatas;
  std::vector<std::shared_ptr<common::ObjectMetadata>> new_mSubObjectMetadatas;
  std::vector<std::shared_ptr<common::TrackedObjectMetadata>>
      new_mTrackedObjectMetadatas;

  if (objectMetadata->mDetectedObjectMetadatas.size()) {
    for (int i = 0; i < objectMetadata->mDetectedObjectMetadatas.size(); i++) {
      std::string name;
      if (type == 0) {
        name = objectMetadata->mSubObjectMetadatas[i]
                   ->mRecognizedObjectMetadatas[0]
                   ->mLabelName;
      } else if (type == 1) {
        name = std::to_string(
            objectMetadata->mTrackedObjectMetadatas[i]->mTrackId);
      }
      if (up_list.find(name) != up_list.end()) {
        new_mDetectedObjectMetadatas.push_back(
            objectMetadata->mDetectedObjectMetadatas[i]);
        if (type == 0) {
          new_mSubObjectMetadatas.push_back(
              objectMetadata->mSubObjectMetadatas[i]);

        } else if (type == 1) {
          new_mTrackedObjectMetadatas.push_back(
              objectMetadata->mTrackedObjectMetadatas[i]);
        }
      }
    }
    objectMetadata->mDetectedObjectMetadatas.clear();
    objectMetadata->mDetectedObjectMetadatas = new_mDetectedObjectMetadatas;
    if (type == 0) {
      objectMetadata->mSubObjectMetadatas.clear();
      objectMetadata->mSubObjectMetadatas = new_mSubObjectMetadatas;
    } else if (type == 1) {
      objectMetadata->mTrackedObjectMetadatas.clear();
      objectMetadata->mTrackedObjectMetadatas = new_mTrackedObjectMetadatas;
    }
    // objectMetadata->mSubObjectMetadatas[0]->mRecognizedObjectMetadatas.clear();
    // for(auto j :mRecognizedObjectMetadatas_)
    // objectMetadata->mSubObjectMetadatas[0]->mRecognizedObjectMetadatas.push_back(j);
  }

  return up_list.size() > 0;
}

bool Filter_Imp::onSegment(const common::Point<int>& p,
                           const common::Point<int>& q,
                           const common::Point<int>& r) {
  return q.mX <= std::max(p.mX, r.mX) && q.mX >= std::min(p.mX, r.mX) &&
         q.mY <= std::max(p.mY, r.mY) && q.mY >= std::min(p.mY, r.mY);
}

int Filter_Imp::orientation(const common::Point<int>& p,
                            const common::Point<int>& q,
                            const common::Point<int>& r) {
  double val =
      1.0 * (q.mY - p.mY) * (r.mX - q.mX) - 1.0 * (q.mX - p.mX) * (r.mY - q.mY);
  if (val == 0) return 0;    // colinear
  return (val > 0) ? 1 : 2;  // clockwise or counterclockwise
}

bool Filter_Imp::doIntersect(const common::Point<int>& p1,
                             const common::Point<int>& q1,
                             const common::Point<int>& p2,
                             const common::Point<int>& q2) {
  int o1 = orientation(p1, q1, p2);
  int o2 = orientation(p1, q1, q2);
  int o3 = orientation(p2, q2, p1);
  int o4 = orientation(p2, q2, q1);

  // General case
  if (o1 != o2 && o3 != o4) return true;

  // Special Cases
  if (o1 == 0 && onSegment(p1, p2, q1)) return true;
  if (o2 == 0 && onSegment(p1, q2, q1)) return true;
  if (o3 == 0 && onSegment(p2, p1, q2)) return true;
  if (o4 == 0 && onSegment(p2, q1, q2)) return true;

  return false;
}

bool Filter_Imp::isPointInsidePolygon(
    const common::Point<int>& p,
    const std::vector<common::Point<int>>& polygon) {
  int n = polygon.size();
  if (n < 3) {
    // IVS_WARN("{0} < 3,point and line can't include a retangle", n);
    return true;
  }
  common::Point<int> extreme = {std::numeric_limits<int>::max(), p.mY};

  int count = 0, i = 0;
  do {
    int next = (i + 1) % n;

    if (doIntersect(polygon[i], polygon[next], p, extreme)) {
      if (orientation(polygon[i], p, polygon[next]) == 0)
        return onSegment(polygon[i], p, polygon[next]);

      count++;
    }
    i = next;
  } while (i != 0);

  return count & 1;
}

bool Filter_Imp::isRectangleInsidePolygon(
    std::vector<common::Point<int>>& rectangle,
    const std::vector<common::Point<int>>& polygon) {
  if (polygon.size() == 0) return true;

  bool flag = true;
  if (polygon.size() < 3) {
    for (const common::Point<int>& corner : polygon) {
      if (isPointInsidePolygon(corner, rectangle)) return true;
    }
    for (int i = 0; i < rectangle.size() && flag; ++i) {
      int nextRectIndex = (i + 1) % rectangle.size();
      common::Point<int> p1 = rectangle[i];
      common::Point<int> q1 = rectangle[nextRectIndex];

      for (int j = 0; j < polygon.size(); ++j) {
        int nextPolyIndex = (j + 1) % polygon.size();
        common::Point<int> p2 = polygon[j];
        common::Point<int> q2 = polygon[nextPolyIndex];

        if ((doIntersect(p1, q1, p2, q2))) return true;
      }
    }
    return false;
  }
  for (const common::Point<int>& corner : rectangle) {
    flag &= isPointInsidePolygon(corner, polygon);
    if (!flag) break;
  }

  for (int i = 0; i < rectangle.size() && flag; ++i) {
    int nextRectIndex = (i + 1) % rectangle.size();
    common::Point<int> p1 = rectangle[i];
    common::Point<int> q1 = rectangle[nextRectIndex];

    for (int j = 0; j < polygon.size(); ++j) {
      int nextPolyIndex = (j + 1) % polygon.size();
      common::Point<int> p2 = polygon[j];
      common::Point<int> q2 = polygon[nextPolyIndex];

      flag &= (!doIntersect(p1, q1, p2, q2));
      if (!flag) break;
    }

    return flag;
  }
}

int64_t Filter::timeToMilliseconds(const std::string& time) {
  int h, m, s;
  char colon;
  std::istringstream timeStream(time);
  timeStream >> h >> colon >> m >> colon >> s;
  return ((h * 3600) + (m * 60) + s) * 1000;
}

REGISTER_WORKER("filter", Filter)
}  // namespace filter
}  // namespace element
}  // namespace sophon_stream