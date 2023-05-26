#pragma once

#include "HungarianState.h"
#include "QualityControl.hpp"
#include "common/ObjectMetadata.h"
// #include "common/Utils.h"
#include "common/Clocker.h"
#include "common/Logger.h"
#include "common/bmnn_utils.h"

namespace sophon_stream {
namespace element {
namespace tracker_sort {

class TransTrack2Output {
 public:
  /**
   * 将跟踪器对象转化为输出对象
   * @param[out]  objs    跟踪对象缓存数组
   * @param[in]   trackId 跟踪id
   * @param[out]  fc      帧对象结果回调
   * @param[in]   topN    前n个最佳结果
   * @return:
   */
  static void transTrack2Output(
      std::vector<std::shared_ptr<common::ObjectMetadata>>& objs,
      const unsigned long long trackId,
      const std::function<void(std::shared_ptr<common::ObjectMetadata>&)>& fc,
      const std::size_t topN, const float maxScore) {
    // IVS_CRITICAL("transTrack2Output");
    // 分值的筛选
    // for (auto iter = objs.begin(); iter != objs.end();) {
    //     if ((*iter)->mTrackedObjectMetadata->mQualityScore < maxScore) {
    //         objs.erase(iter);
    //     } else iter++;
    // }

    // 排序
    std::sort(objs.begin(), objs.end(), sortByScore);
    int n = 0;
    int limit = std::min(objs.size(), topN);
    for (auto& obj : objs) {
      if (n < topN) {
        obj->mTrackedObjectMetadata->mTrackId = trackId;
        // get current date
        std::string strDay;
        Clocker::getCurrentUs(obj->mTrackedObjectMetadata->mCaptureTime,
                              strDay);
        // obj->mTrackedObjectMetadata->mImagePath =  strDay + "/" +
        // Utils::genUUID() + ".jpg";
        if (n == (limit - 1)) {
          obj->mTrackedObjectMetadata->mTrackFlag = common::TrLast;
        }
        // IVS_CRITICAL("track image path: {0}",
        // obj->mTrackedObjectMetadata->mImagePath);
        fc(obj);
        n++;
      } else
        break;
    }
  }

 private:
  /**
   * 按照box的得分从大到小进行比较
   * @param[in]  v1  跟踪对象(submentadata)
   * @param[in]  v2  跟踪对象(submentadata)
   * @return:从大到小
   */
  static bool sortByScore(const std::shared_ptr<common::ObjectMetadata>& v1,
                          const std::shared_ptr<common::ObjectMetadata>& v2) {
    return v1->mTrackedObjectMetadata->mQualityScore >
           v2->mTrackedObjectMetadata->mQualityScore;
  }
};

// 跟踪器类:针对一个目标的跟踪
class Tracker {
 public:
  // 单个物体的metadata,也就是有box的一级,类似于单个人脸
  using PUT_TASK_FUNC =
      std::function<void(std::shared_ptr<common::ObjectMetadata>&)>;
  /**
   * Tracker
   * @param[in]   rect    box
   * @param[in]   trackId 跟踪id
   * @param[in]   topN    前n个最佳结果
   * @return:
   */
  Tracker(cv::Rect rect, unsigned long long& trackId, const int topN)
      : mTopN(topN) { /*:mTrackerId(&trackId)*/
    this->rect = rect;

    mTrackerId = sTrackId;
    sTrackId++;
  }

  ~Tracker() {}

  /**
   * 获取跟踪器中的目标(统计出现次数)
   * @return:     跟踪器中的目标
   */
  cv::Rect2f GetRectPrediction() {
    if (mTimeSinceUpdate > 0) mHitStreak = 0;
    mTimeSinceUpdate++;
    return rect;
  }

  /**
   * 更新跟踪器中的目标
   * @param[in]   rect    目标box
   * @param[in]   dataCorrect   目前未使用
   * @return:
   */
  void Update(cv::Rect2f rect, bool dataCorrect) {
    this->rect = rect;
    mHitStreak++;
    mTimeSinceUpdate = 0;
  }

  /**
   * 获取跟踪器中的目标(不统计出现次数)
   * @return:     跟踪器中的人脸
   */
  cv::Rect2f getState() {
    return cv::Rect2f(static_cast<int>(rect.x), static_cast<int>(rect.y),
                      static_cast<int>(rect.width),
                      static_cast<int>(rect.height));
  }

  /**
   * 获取跟踪id
   * @return: 跟踪id
   */
  inline unsigned long long getTrackId() { return mTrackerId; }

  /**
   * 统计update后跟踪器经历了几帧
   * @return: mTimeSinceUpdate
   */
  inline int getTimeSinceUpdate() { return mTimeSinceUpdate; }

  /**
   * 统计跟踪器连续做了几次update
   * @return: mHitStreak
   */
  inline unsigned long long getHitStreak() { return mHitStreak; }

  /**
   * 获取当前跟踪器所有的跟踪对象
   * @return: mObjs
   */
  inline std::vector<std::shared_ptr<common::ObjectMetadata>>&
  getTrackObject() {
    return mObjs;
  }

  inline void clearObjs() { mObjs.clear(); }

  /**
   * 根据时间戳过滤目标
   * @param[in]   timebase      时间戳
   * @param[in]   first         是否第一次出现 true为第一次
   * @param[in]   metadata      目标
   * @param[in]   fc            帧对象的回调函数
   * @return:
   */

  void timebaseFilter(const long timebase, const bool first,
                      std::shared_ptr<common::ObjectMetadata>& metadata,
                      const PUT_TASK_FUNC fc, const float maxScore,
                      const std::int64_t stamp) {
    // const std::int64_t stamp = metadata->getTimestamp();
    if (!first) mTimer += (stamp - mLastTime);

    mLastTime = stamp;

    // IVS_CRITICAL("mTimer:{0}----timebase:{1}", mTimer, timebase);

    if ((mTimer > timebase) && (!first)) {
      pushbackObjs(metadata);
      TransTrack2Output::transTrack2Output(mObjs, mTrackerId, fc, mTopN,
                                           maxScore);

      mTimer = 0;
      mObjs.clear();
    } else {
      pushbackObjs(metadata);
    }
  }

 private:
  int mTopN = 1;

  std::int64_t mTimer = 0;
  std::int64_t mLastTime = 0;
  std::vector<std::shared_ptr<common::ObjectMetadata>> mObjs;

  cv::Rect2f rect;
  static unsigned long long sTrackId;
  unsigned long long mTrackerId = 0;
  int mTimeSinceUpdate = 0;
  unsigned long long mHitStreak = 0;

  /**
   * 当前跟踪器所有的跟踪对象的末尾插入一个元素
   * @param[in]   metData        跟踪目标
   * @return:
   */
  void pushbackObjs(std::shared_ptr<common::ObjectMetadata>& metData) {
    if (mObjs.size() == 0) {
      mObjs.push_back(metData);
    } else {
      if (metData->mTrackedObjectMetadata->mQualityScore >
          mObjs[0]->mTrackedObjectMetadata->mQualityScore) {
        mObjs.clear();
        mObjs.push_back(metData);
      }
    }
  }
};

// 通道跟踪类:每一路摄像头下包含多个跟踪器
class TrackerChannel {
 public:
  // 单帧的mentdata,下面包含多个submentdata(单个跟踪器)
  using PUT_TASK_FUNC = std::function<void(
      std::shared_ptr<common::ObjectMetadata>&)>;  // 总的metadata

  /**
   * TrackerChannel
   * @param[in]   qc      质量控制对象
   * @param[in]   topN    前n个最佳结果
   * @param[in]   iouThreshold  iou阈值
   * @param[in]   maxAge        update后Tracker跟踪器经历的最大帧数
   * @param[in]   minHits       Tracker跟踪器连续做update的最大次数
   * @param[in]   updateTimes   目前未用
   * @return:
   */

  TrackerChannel(const QualityControl& qc, const int topN,
                 const float iouThreshold, const int maxAge = 1,
                 const unsigned long long minHits = 3,
                 const int updateTimes = 0)
      : mQualityControl(qc),
        mTopN(topN),
        mIouThreshold(iouThreshold),
        mMaxAge(maxAge),
        mMinHits(minHits) {}
  ~TrackerChannel() { mListTrackers.clear(); }

  void clearBuffer() {
    for (auto& obj : mListTrackers) {
      obj.clearObjs();
    }
  }

  /**
   * update函数
   * 将每一个目标做了质量评价，放入跟踪其中，新的目标和跟踪器中旧的目标做了
   * iou,并根据iou的结果进行匈牙利匹配。
   * 如果匹配为同人，旧的跟踪器做update;如果为新人,创建一个新的跟踪器，
   * 当Tracker跟踪器经历的最大帧数超过maxAge或者人脸超出视频区域，视为人脸消失，删除
   * 该跟踪器
   * @param[in]   timebase 时间戳
   * @param[in]   frame    前n个最佳结果
   * @param[in]   fc       帧对象的回调函数
   * @return:
   */

  void update(const long timebase,
              std::shared_ptr<common::ObjectMetadata>& frameMetdata,
              const PUT_TASK_FUNC& fc) {
    mFrameCount++;

    for (auto iter = frameMetdata->mSubObjectMetadatas.begin();
         iter != frameMetdata->mSubObjectMetadatas.end();) {
      // 转换坐标
      std::vector<cv::Point2f> landmarks;
      // for(auto& element:(*iter)->mSpDataInformation->mKeyPoints){
      //     cv::Point2f temp;
      //     temp.x = element.second.mPoint.mX;
      //     temp.y = element.second.mPoint.mY;
      //     landmarks.push_back(temp);
      // }
      cv::Rect2f rect;
      // rect.x = (*iter)->mSpDataInformation->mBox.mX;
      // rect.y = (*iter)->mSpDataInformation->mBox.mY;
      // rect.width = (*iter)->mSpDataInformation->mBox.mWidth;
      // rect.height = (*iter)->mSpDataInformation->mBox.mHeight;

      rect.x = (*iter)->mDetectedObjectMetadata->mBox.mX;
      rect.y = (*iter)->mDetectedObjectMetadata->mBox.mY;
      rect.width = (*iter)->mDetectedObjectMetadata->mBox.mWidth;
      rect.height = (*iter)->mDetectedObjectMetadata->mBox.mHeight;

      // 计算
      (*iter)->mTrackedObjectMetadata =
          std::make_shared<common::TrackedObjectMetadata>();
      // 这步没有landmarks
      mQualityControl.judgeQualityFace(
          landmarks, rect, (*iter)->mTrackedObjectMetadata->mQualityScore);
      IVS_DEBUG("face score:{0}",
                (*iter)->mTrackedObjectMetadata->mQualityScore);
      iter++;
    }

    std::vector<std::vector<float>> trks(mListTrackers.size());
    for (int i = 0; i < trks.size(); i++) {
      trks[i].resize(5, 0.0f);
      cv::Rect2f&& pos = mListTrackers[i].GetRectPrediction();
      trks[i] = {pos.x, pos.y, pos.br().x, pos.br().y, 0.0f};
    }

    for (auto iter = trks.begin(); iter != trks.end();) {
      if (std::isfinite((*iter)[0]) && std::isfinite((*iter)[1]) &&
          std::isfinite((*iter)[2]) && std::isfinite((*iter)[3]) &&
          std::isfinite((*iter)[4])) {
        iter++;
      } else
        trks.erase(iter);
    }

    if (frameMetdata->mSubObjectMetadatas.size() != 0) {
      //            PayloadVideoFaceRecognize *payload =
      //            dynamic_cast<PayloadVideoFaceRecognize
      //            *>(frame->mPayload.get()); if (payload != nullptr) {
      //                if (!payload->mliveStream) {
      //                    IVS_DEBUG("SortAlgorithm::update begin hungarian...
      //                    trakerList size:{0}", mListTrackers.size());
      //                }
      //            }

      std::vector<std::vector<int>> matches;
      std::vector<int> unmatchedDets;
      std::vector<int> unmatchedTrks;
      // iou and hungarian
      associateDetectionsToTrackers(frameMetdata->mSubObjectMetadatas, trks,
                                    matches, unmatchedDets, unmatchedTrks,
                                    mIouThreshold);

      for (int i = 0; i < mListTrackers.size(); i++) {
        if (std::find(unmatchedTrks.begin(), unmatchedTrks.end(), i) ==
            unmatchedTrks.end()) {
          int d = -1;
          // 找到matches第2列==i的matches行数x
          // 取matches[x,0]为d
          // 因此d为跟踪器i匹配到的对应检测框下标
          for (auto& match : matches) {
            if (match[1] == i) {
              d = match[0];
              break;
            }
          }
          if (d != -1) {
            frameMetdata->mSubObjectMetadatas[d]
                ->mTrackedObjectMetadata->mTrackId =
                mListTrackers[i].getTrackId();
            frameMetdata->mSubObjectMetadatas[d]
                ->mTrackedObjectMetadata->mTrackerFilter = false;

            cv::Rect2f rectBox;
            rectBox.x = frameMetdata->mSubObjectMetadatas[d]
                            ->mDetectedObjectMetadata->mBox.mX;
            rectBox.y = frameMetdata->mSubObjectMetadatas[d]
                            ->mDetectedObjectMetadata->mBox.mY;
            rectBox.width = frameMetdata->mSubObjectMetadatas[d]
                                ->mDetectedObjectMetadata->mBox.mWidth;
            rectBox.height = frameMetdata->mSubObjectMetadatas[d]
                                 ->mDetectedObjectMetadata->mBox.mHeight;
            mListTrackers[i].Update(rectBox, true);
            mListTrackers[i].timebaseFilter(
                timebase, false, frameMetdata->mSubObjectMetadatas[d], fc,
                mQualityControl.getConfig().maxScore,
                frameMetdata->mFrame->mTimestamp);
          }
        }
      }

      for (auto& i : unmatchedDets) {
        cv::Rect2f rectBox;
        rectBox.x = frameMetdata->mSubObjectMetadatas[i]
                        ->mDetectedObjectMetadata->mBox.mX;
        rectBox.y = frameMetdata->mSubObjectMetadatas[i]
                        ->mDetectedObjectMetadata->mBox.mY;
        rectBox.width = frameMetdata->mSubObjectMetadatas[i]
                            ->mDetectedObjectMetadata->mBox.mWidth;
        rectBox.height = frameMetdata->mSubObjectMetadatas[i]
                             ->mDetectedObjectMetadata->mBox.mHeight;
        auto&& trk = Tracker(rectBox, mTrackId, mTopN);
        frameMetdata->mSubObjectMetadatas[i]->mTrackedObjectMetadata->mTrackId =
            trk.getTrackId();
        frameMetdata->mSubObjectMetadatas[i]
            ->mTrackedObjectMetadata->mTrackerFilter = false;
        trk.timebaseFilter(timebase, true, frameMetdata->mSubObjectMetadatas[i],
                           fc, mQualityControl.getConfig().maxScore,
                           frameMetdata->mFrame->mTimestamp);
        mListTrackers.emplace_back(trk);
      }
    }

    for (auto iter = mListTrackers.rbegin(); iter != mListTrackers.rend();) {
      if (frameMetdata == nullptr) {
        iter++;
        continue;
      }

      cv::Rect2f&& rectTracker = iter->getState();

      if (frameMetdata->mFrame->mSpData == nullptr) {
        if (frameMetdata->mFrame->mEndOfStream) {
          TransTrack2Output::transTrack2Output(
              iter->getTrackObject(), iter->getTrackId(), fc, mTopN,
              mQualityControl.getConfig().maxScore);
          // IVS_INFO("ID{0} will be removed!", iter->getTrackId());
          iter = std::vector<Tracker>::reverse_iterator(
              mListTrackers.erase((++iter).base()));
        } else
          iter++;
        continue;
      }

      // IVS_TRACE("rectTracker.x:{0}---orig width:{1}", rectTracker.x,
      // frameMetdata->mFrame->mWidth);
      if (iter->getTimeSinceUpdate() >= mMaxAge || rectTracker.x < 0 ||
          rectTracker.y < 0 ||
          rectTracker.br().x > frameMetdata->mFrame->mWidth ||
          rectTracker.br().y > frameMetdata->mFrame->mHeight ||
          frameMetdata->mFrame->mEndOfStream &&
              iter->getHitStreak() >= mMinHits) {
        TransTrack2Output::transTrack2Output(
            iter->getTrackObject(), iter->getTrackId(), fc, mTopN,
            mQualityControl.getConfig().maxScore);
        // IVS_INFO("ID{0} will be removed!", iter->getTrackId());
        iter = std::vector<Tracker>::reverse_iterator(
            mListTrackers.erase((++iter).base()));
      } else
        iter++;
    }
  }

 private:
  QualityControl mQualityControl;
  float mIouThreshold = 0.25;
  int mMaxAge = 0;
  int mUpdateTimes = 0;
  unsigned long long mMinHits = 0;
  unsigned long long mFrameCount = 0;
  std::vector<Tracker> mListTrackers;

  unsigned long long mTrackId = 0;

  int mTopN = 1;

 private:
  /**
   * iou函数 求两个人脸矩形的iou值
   * 公式为：相交面积/(两个面积的和-相交面积）
   * @param[in]   det  新检测到的人脸框
   * @param[in]   trk  跟踪器中的人脸框
   * @return:相交面积/(两个面积的和-相交面积）   比例
   */
  float iou(const cv::Rect2f& det, const std::vector<float>& trk) {
    float xx1 = std::max(det.x, trk[0]);
    float yy1 = std::max(det.y, trk[1]);
    float xx2 = std::min(det.br().x, trk[2]);
    float yy2 = std::min(det.br().y, trk[3]);
    float width = std::max(0.0f, xx2 - xx1);
    float height = std::max(0.0f, yy2 - yy1);
    float areaOverlap = width * height;
    return (float)areaOverlap /
           (det.width * det.height + (trk[2] - trk[0]) * (trk[3] - trk[1]) -
            areaOverlap);
  }

  /**
   * associateDetectionsToTrackers函数
   * 检测到的所有目标框和所有跟踪器中的人脸框先做iou，然后 根据iou进行匈牙利匹配
   * @param[in]   dets  新检测到的所有人脸
   * @param[in]   trks  所有跟踪器中的人脸框
   * @param[out]   matches  匹配到的人脸id数组
   * @param[out]   unmatchedDets  为匹配到的检测人脸id数组
   * @param[out]   unmatchedTrks  匹配到的跟踪器id数组
   * @param[in]   iouThreshold  iou阈值
   * @return:
   */
  void associateDetectionsToTrackers(
      const std::vector<std::shared_ptr<common::ObjectMetadata>>& dets,
      const std::vector<std::vector<float>>& trks,
      std::vector<std::vector<int>>& matches, std::vector<int>& unmatchedDets,
      std::vector<int>& unmatchedTrks, const float iouThreshold = 0.25) {
    if (trks.size() == 0) {
      for (std::size_t i = 0; i < dets.size(); i++) {
        unmatchedDets.push_back(i);
      }
      return;
    }
    std::size_t rows = dets.size();
    std::size_t cols = trks.size();
    std::vector<std::vector<float>> iouMatrix;
    iouMatrix.resize(rows);
    for (std::size_t i = 0; i < rows; i++) {
      iouMatrix[i].resize(cols);
    }

    for (std::size_t i = 0; i < rows; i++) {
      for (std::size_t j = 0; j < cols; j++) {
        // iouMatrix[i][j] = -iou(dets[i].mCropBox, trks[j]);

        cv::Rect2f rectBox;
        // rectBox.x = dets[i]->mSpDataInformation->mBox.mX;
        // rectBox.y = dets[i]->mSpDataInformation->mBox.mY;
        // rectBox.width = dets[i]->mSpDataInformation->mBox.mWidth;
        // rectBox.height = dets[i]->mSpDataInformation->mBox.mHeight;
        rectBox.x = dets[i]->mDetectedObjectMetadata->mBox.mX;
        rectBox.y = dets[i]->mDetectedObjectMetadata->mBox.mY;
        rectBox.width = dets[i]->mDetectedObjectMetadata->mBox.mWidth;
        rectBox.height = dets[i]->mDetectedObjectMetadata->mBox.mHeight;
        iouMatrix[i][j] = -iou(rectBox, trks[j]);
      }
    }

    std::vector<std::vector<int>> matchedIndices = linear_assignment(iouMatrix);

    // fixme:vector 元素转负数有没有更好的方法
    for (std::size_t i = 0; i < rows; i++) {
      for (std::size_t j = 0; j < cols; j++) {
        iouMatrix[i][j] = -iouMatrix[i][j];
      }
    }

    for (std::size_t i = 0; i < rows; i++) {
      bool foundInMatched = false;
      for (std::size_t j = 0; j < matchedIndices.size(); j++) {
        if (i == matchedIndices[j][0]) {
          foundInMatched = true;
          break;
        }
      }
      if (!foundInMatched) unmatchedDets.push_back(i);
    }
    for (std::size_t i = 0; i < cols; i++) {
      bool foundInMatched = false;
      for (std::size_t j = 0; j < matchedIndices.size(); j++) {
        if (i == matchedIndices[j][1]) {
          foundInMatched = true;
          break;
        }
      }
      if (!foundInMatched) unmatchedTrks.push_back(i);
    }
    for (auto& m : matchedIndices) {
      if (iouMatrix[m[0]][m[1]] < iouThreshold) {
        unmatchedDets.push_back(m[0]);
        unmatchedTrks.push_back(m[1]);
      } else {
        matches.push_back(m);
      }
    }
  }
};
}  // namespace tracker_sort
}  // namespace element
}  // namespace sophon_stream