#include "sort.hpp"

#include <nlohmann/json.hpp>
// 跟踪的json参数
#define JSON_ALGORITHM_TRACK_TOPN "track_TopN"
#define JSON_ALGORITHM_TRACK_IOU "track_Iou"
#define JSON_ALGORITHM_TRACK_MAX_AGE "track_MaxAge"
#define JSON_ALGORITHM_TRACK_MIN_HINS "track_MinHins"
#define JSON_ALGORITHM_TRACK_UPDATE_TIMES "track_UpdateTimes"
#define JSON_ALGORITHM_TRACK_BASE_TIMES "track_BaseTimes"

////质量判断的json参数
#define JSON_ALGORITHM_QUALITY_THRES_AREA "quality_ta"
#define JSON_ALGORITHM_QUALITY_THRES_RATIOLOWBOUND "quality_trl"
#define JSON_ALGORITHM_QUALITY_THRES_RATIOUPBOUND "quality_tru"
#define JSON_ALGORITHM_QUALITY_WIDTH "quality_w"
#define JSON_ALGORITHM_QUALITY_HEIGHT "quality_h"
#define JSON_ALGORITHM_QUALITY_MARGIN "quality_mg"
// #define JSON_ALGORITHM_QUALITY_BOX_WEIGHT "quality_bw"
// #define JSON_ALGORITHM_QUALITY_CENTER_WEIGHT "quality_cw"
// #define JSON_ALGORITHM_QUALITY_SIDE_WEIGHT "quality_sw"
#define JSON_ALGORITHM_QUALITY_MAX_QUSCORE "quality_maxSc"
#define JSON_ALGORITHM_QUALITY_lATERAL_SIDE "quality_ls"

namespace sophon_stream {
namespace algorithm {
namespace tracker_sort {

TrackerChannels::TrackerChannels() {}

TrackerChannels::~TrackerChannels() {}

/**
 * 初始化多路的跟踪器:跟踪器和质量评估模块的初始化参数
 * @param[in]   json      json配置
 * @return: 成功或者失败
 */
common::ErrorCode TrackerChannels::init(const std::string& json) {
  common::ErrorCode errorCode = common::ErrorCode::SUCCESS;
  do {
    auto configure = nlohmann::json::parse(json, nullptr, false);
    if (!configure.is_object()) {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    // auto model_list = configure.find("models");
    // for(auto modelConfigure : *model_list)
    // {
    // 解析 trakcer的参数
    auto topNCon = configure.find(JSON_ALGORITHM_TRACK_TOPN);
    if (configure.end() != topNCon && topNCon->is_number_integer()) {
      mTopN = topNCon->get<int>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto IOUCon = configure.find(JSON_ALGORITHM_TRACK_IOU);
    if (configure.end() != IOUCon && IOUCon->is_number_float()) {
      mIou = IOUCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto maxAgeCon = configure.find(JSON_ALGORITHM_TRACK_MAX_AGE);
    if (configure.end() != maxAgeCon && maxAgeCon->is_number_integer()) {
      mMaxAge = maxAgeCon->get<int>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto minHinsCon = configure.find(JSON_ALGORITHM_TRACK_MIN_HINS);
    if (configure.end() != minHinsCon && minHinsCon->is_number_integer()) {
      mMinHins = minHinsCon->get<int>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto updateTimesCon = configure.find(JSON_ALGORITHM_TRACK_UPDATE_TIMES);
    if (configure.end() != updateTimesCon &&
        updateTimesCon->is_number_integer()) {
      mUpdateTimes = updateTimesCon->get<int>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    auto baseTimesCon = configure.find(JSON_ALGORITHM_TRACK_BASE_TIMES);
    if (configure.end() != baseTimesCon && baseTimesCon->is_number_integer()) {
      mTimebase = baseTimesCon->get<int>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    //   IVS_DEBUG("maxage:{0}-iou:{1}-minHins:{2}-updateTimes:{3}", mMaxAge,
    //   mIou, mMinHins, mUpdateTimes);
    // 解析质量判断的参数
    float tempThresArea;
    auto thresAreaCon = configure.find(JSON_ALGORITHM_QUALITY_THRES_AREA);
    if (configure.end() != thresAreaCon && thresAreaCon->is_number_float()) {
      tempThresArea = thresAreaCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempThresRationLB;
    auto thresRLBCon =
        configure.find(JSON_ALGORITHM_QUALITY_THRES_RATIOLOWBOUND);
    if (configure.end() != thresRLBCon && thresRLBCon->is_number_float()) {
      tempThresRationLB = thresRLBCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempThresRationUB;
    auto thresRUBCon =
        configure.find(JSON_ALGORITHM_QUALITY_THRES_RATIOUPBOUND);
    if (configure.end() != thresRUBCon && thresRUBCon->is_number_float()) {
      tempThresRationUB = thresRUBCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempWidth;
    auto widthCon = configure.find(JSON_ALGORITHM_QUALITY_WIDTH);
    if (configure.end() != widthCon && widthCon->is_number_float()) {
      tempWidth = widthCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempHeight;
    auto heightCon = configure.find(JSON_ALGORITHM_QUALITY_HEIGHT);
    if (configure.end() != heightCon && heightCon->is_number_float()) {
      tempHeight = heightCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempMargin;
    auto marginCon = configure.find(JSON_ALGORITHM_QUALITY_MARGIN);
    if (configure.end() != marginCon && marginCon->is_number_float()) {
      tempMargin = marginCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempMaxScore;
    auto maxScoreCon = configure.find(JSON_ALGORITHM_QUALITY_MAX_QUSCORE);
    if (configure.end() != maxScoreCon && maxScoreCon->is_number_float()) {
      tempMaxScore = maxScoreCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }

    float tempLateralSide;
    auto lateralSideCon = configure.find(JSON_ALGORITHM_QUALITY_lATERAL_SIDE);
    if (configure.end() != lateralSideCon &&
        lateralSideCon->is_number_float()) {
      tempLateralSide = lateralSideCon->get<float>();
    } else {
      errorCode = common::ErrorCode::PARSE_CONFIGURE_FAIL;
      break;
    }
    QualityConfig tempCon(tempThresArea, tempThresRationLB, tempThresRationUB,
                          tempWidth, tempHeight, tempMargin, tempMaxScore,
                          tempLateralSide);
    pQualityControl = std::make_shared<QualityControl>(tempCon);
    // }

  } while (false);

  if (common::ErrorCode::SUCCESS != errorCode) {
    uninit();
  }

  return errorCode;
}

/**
 * 跟踪器进行跟踪
 * @param[in]   objectMetadata      单帧的mentdata数据
 * @param[in]   fc    回调函数
 */
void TrackerChannels::process(
    std::shared_ptr<common::ObjectMetadata> objectMetadata,
    const PUT_TASK_FUNC& fc) {
  auto&& iter = mSorts.find(objectMetadata->mFrame->mChannelId);
  if (iter == mSorts.end()) {
    // 没找到跟踪器，则创建一个跟踪器，进行跟踪,并添加到map中
    std::shared_ptr<TrackerChannel> sort = std::make_shared<TrackerChannel>(
        *pQualityControl, mTopN, mIou, mMaxAge, mMinHins, mUpdateTimes);
    sort->update(mTimebase, objectMetadata, fc);
    mSorts.insert(std::make_pair(objectMetadata->mFrame->mChannelId, sort));
  } else {
    // 找到了跟踪器，就做跟踪
    iter->second->update(mTimebase, objectMetadata, fc);
  }
  // 最后一帧,非ok帧
  if (objectMetadata->mFrame->mEndOfStream) {
    clearBuffer();
  }
}

/**
 * 反初始化函数
 */
void TrackerChannels::uninit() {}

/**
 * 清除跟踪器的缓冲
 */
void TrackerChannels::clearBuffer() {
  for (auto& tracker : mSorts) {
    tracker.second->clearBuffer();
  }
}

/**
 * 删除对应ID的跟踪器
 * @param[in]   taskID      跟踪器的ID
 */
void TrackerChannels::removeTracker(const int& taskId) {
  auto&& iter = mSorts.find(taskId);
  if (iter != mSorts.end()) {
    mSorts.erase(iter);
  }
}

}  // namespace tracker_sort
}  // namespace algorithm
}  // namespace sophon_stream
