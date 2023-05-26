#pragma once

#include <math.h>

#include "opencv2/opencv.hpp"
#define EPSINON 0.00001f

#define DIVISOR 3

namespace sophon_stream {
namespace algorithm {
namespace tracker_sort {

struct QualityConfig {
  QualityConfig(const float ta, const float trl, const float tru, const float w,
                const float h, const float mg, const float maxSc,
                const float ls)
      : thresArea(ta),
        thresRatioLowBound(trl),
        thresRatioUpBound(tru),
        width(w),
        height(h),
        center(width / 2, height / 2),
        margin(mg),
        maxScore(maxSc),
        lateralSide(ls) {
    float upArea = pow(lateralSide, 2);
    float lowArea = thresArea;
    unit = (upArea - lowArea) / 100;
  }

  float thresArea;          /**< 面积阈值(h*w) */
  float thresRatioLowBound; /**< 比率阈值下限(h/w) */
  float thresRatioUpBound;  /**< 比率阈值上限(h/w) */

  float lateralSide; /**< std::min(W,H) W为视频宽度 H为视频高度 边长上限 */
  float
      unit; /**< 单位(需要换算到0.0f~100.0f) 通过(upArea-lowArea)/100计算得出 */

  float width;
  float height;
  cv::Point2f center;
  float margin;
  float maxScore = 50.0f;
};

class QualityControl {
 public:
  QualityControl(const QualityConfig& config) : mQualityConfig(config) {}

  /**
   * judgeQualityFace函数 检测到的所有人脸框和所有跟踪器中的人脸框先做iou，然后
   * 根据iou进行匈牙利匹配
   * @param[in]   landmarks  人脸眼位嘴位
   * @param[in]   rectFace   人脸框
   * @param[out]   qualityScore  质量比分
   * @return: 是否超出了margin范围
   */
  bool judgeQualityFace(const std::vector<cv::Point2f>& landmarks,
                        const cv::Rect2f& rectFace, float& qualityScore) {
    return false;

    bool ret = false;
    bool box = false;
    bool ld = false;
    bool hi = false;
    bool ang = false;
    boxFilter(rectFace, box);
    centerScore(rectFace, ret);
    ldValid(landmarks, rectFace, ld);
    float vertiScore = verticalDis(landmarks, hi);
    float angleScore = includeAngle(landmarks, ang);
    if (ret && box && ld && hi && ang) {
      qualityScore = 100 * (vertiScore + angleScore) / 2;
    } else {
      qualityScore = 0.0;
    }

    return ret && box && ld && hi && ang;
  }

  const QualityConfig& getConfig() { return mQualityConfig; }

 private:
  QualityConfig mQualityConfig;
  /**
   * 根据框的面积 长宽比 输出一个比分
   * @param[in]  rect  人脸框
   * @return 比分0.0f~100.0f
   *
   */
  float boxFilter(const cv::Rect2f& rect, bool& outRange) {
    float area = rect.width * rect.height;
    float ratio = rect.height / rect.width;
    if (area < mQualityConfig.thresArea) {
      outRange = false;
      return 0.0f;
    } else if (ratio < mQualityConfig.thresRatioLowBound ||
               ratio > mQualityConfig.thresRatioUpBound) {
      outRange = false;
      return 0.0f;
    } else {
      outRange = true;
      return (area - mQualityConfig.thresArea) / mQualityConfig.unit;
    }
  }

  /**
   * 根据人脸框中心点坐标和原图中心点坐标的距离 输出一个比分
   * @param[in]  rect  人脸框
   * @return 比分0.0f~100.0f
   *
   */
  float centerScore(const cv::Rect2f& rect, bool& outRange) {
    cv::Point2f point(rect.x + rect.width / 2, rect.y + rect.height / 2);
    float score = (1 - (pow((point.x - mQualityConfig.center.x), 2) +
                        pow((point.y - mQualityConfig.center.y), 2)) /
                           (pow(mQualityConfig.center.x, 2) +
                            pow(mQualityConfig.center.y, 2))) *
                  100;
    if (point.y <= (mQualityConfig.height / 2)) {  // 上部
      score *= 0.8f;
    }  // else为下部

    (rect.tl().x > mQualityConfig.margin &&
     rect.tl().y > mQualityConfig.margin &&
     rect.br().x < (mQualityConfig.width - mQualityConfig.margin) &&
     rect.br().y < (mQualityConfig.height - mQualityConfig.margin))
        ? (outRange = true)
        : (outRange = false);

    return score;
  }

  /**
   * 根据ld和box的坐标,判断ld是否在box内部
   * @param[in]  landmarks  人脸眼位嘴位
   * @param[in]  rect  box坐标(cx,xy,w,h)
   * @param[out]  outRange  是否超边界
   *
   */
  void ldValid(const std::vector<cv::Point2f>& landmarks,
               const cv::Rect2f& rect, bool& outRange) {
    int xMinMax[2] = {1000, 0};
    int yMinMax[2] = {1000, 0};
    // land 坐标关系

    // box和ld的关系
    for (unsigned i = 0; i < landmarks.size(); i++) {
      if (landmarks[i].x < xMinMax[0]) {
        xMinMax[0] = landmarks[i].x;
      } else if (landmarks[i].x > xMinMax[1]) {
        xMinMax[1] = landmarks[i].x;
      }

      if (landmarks[i].y < yMinMax[0]) {
        yMinMax[0] = landmarks[i].y;
      } else if (landmarks[i].y > yMinMax[1]) {
        yMinMax[1] = landmarks[i].y;
      }
    }
    outRange = true;
    cv::Point2f pointRD(rect.x + rect.width - 1, rect.y + rect.height - 1);
    if ((xMinMax[0] < rect.x + 1) || (xMinMax[1] > pointRD.x - 1) ||
        (yMinMax[0] < rect.y + 1) || (yMinMax[1] > pointRD.y - 1)) {
      outRange = false;
      // return;
    }
    if (outRange == true) {
      // ld中鼻尖 是否在其他四点内部
      xMinMax[0] = 1000;
      xMinMax[1] = 0;
      yMinMax[0] = 1000;
      yMinMax[1] = 0;
      for (unsigned i = 0; i < landmarks.size(); i++) {
        if (i == 2) continue;

        if (landmarks[i].x < xMinMax[0]) {
          xMinMax[0] = landmarks[i].x;
        } else if (landmarks[i].x > xMinMax[1]) {
          xMinMax[1] = landmarks[i].x;
        }

        if (landmarks[i].y < yMinMax[0]) {
          yMinMax[0] = landmarks[i].y;
        } else if (landmarks[i].y > yMinMax[1]) {
          yMinMax[1] = landmarks[i].y;
        }
      }
      if ((xMinMax[0] >= landmarks[2].x) || (xMinMax[1] <= landmarks[2].x) ||
          (yMinMax[0] >= landmarks[2].y) || (yMinMax[1] <= landmarks[2].y)) {
        outRange = false;
      } else {
        outRange = true;
      }
    }
  }

  /**
   * 根据ld和box的坐标,判断ld是否在box内部
   * @param[in]  landmarks  人脸眼位嘴位
   * @param[in]  rect  box坐标(cx,xy,w,h)
   * @param[out]  outRange  是否超边界
   *
   */
  float verticalDis(const std::vector<cv::Point2f>& landmarks, bool& outRange) {
    // float result = 0;
    std::vector<cv::Point2f> vecs;
    // 0,3,4,1 vecs
    cv::Point2f temp = landmarks[0] - landmarks[2];
    vecs.push_back(temp);
    temp = landmarks[3] - landmarks[2];
    vecs.push_back(temp);
    temp = landmarks[4] - landmarks[2];
    vecs.push_back(temp);
    temp = landmarks[1] - landmarks[2];
    vecs.push_back(temp);
    // areas
    float areas = 0;
    std::vector<float> hghts;
    int b = 1;
    for (unsigned int i = 0; i < 4; i++) {
      if (i == 3) {
        b = 0;
      } else {
        b = i + 1;
      }
      float area = (vecs[i].y * vecs[b].x - vecs[i].x * vecs[b].y) / 2.0;
      float side = sqrt(pow((vecs[i].x - vecs[b].x), 2) +
                        pow((vecs[i].y - vecs[b].y), 2));
      float height = area / side;
      hghts.push_back(height);
      areas = areas + area;
    }
    areas = sqrt(areas);
    for (unsigned int i = 0; i < 4; i++) {
      hghts[i] = hghts[i] * 1.0 / areas;
    }
    // 左右
    float sx =
        (hghts[0] < hghts[2]) ? (hghts[0] / hghts[2]) : (hghts[2] / hghts[0]);
    // 上下
    float sy =
        (hghts[1] < hghts[3]) ? (hghts[1] / hghts[3]) : (hghts[3] / hghts[1]);
    // 有效性判断 越小 越宽松
    float lrThreshold = 0.3;  // 0.5  0.3
    float udThreshold = 0.3;  // 0.75 0.3
    float score = 0.0;
    if ((sx < lrThreshold) || (sy < udThreshold)) {
      outRange = false;
      return score;
    }
    // 得分
    float sxScore = (sx - lrThreshold) * 1.0 / (1.0 - lrThreshold);
    float syScore = (sy - udThreshold) * 1.0 / (1.0 - udThreshold);
    score = (sxScore + syScore) * 1.0 / 2.0;
    outRange = true;
    return score;
  }

  /**
   * 根据ld和box的坐标,判断ld是否在box内部
   * @param[in]  landmarks  人脸眼位嘴位
   * @param[in]  rect  box坐标(cx,xy,w,h)
   * @param[out]  outRange  是否超边界
   *
   */
  float includeAngle(const std::vector<cv::Point2f>& landmarks,
                     bool& outRange) {
    std::vector<cv::Point2f> dist;
    for (unsigned int i = 0; i < 5; i++) {
      cv::Point2f temp;
      if (i == 2) continue;
      temp = landmarks[i] - landmarks[2];
      dist.push_back(temp);
    }
    // 上下
    std::vector<float> alpha;
    for (unsigned int i = 0; i < 3; i = i + 2) {
      float dot = dist[i].x * dist[i + 1].x + dist[i].y * dist[i + 1].y;
      float mul = sqrt(pow(dist[i].x, 2) + pow(dist[i].y, 2)) *
                  sqrt(pow(dist[i + 1].x, 2) + pow(dist[i + 1].y, 2));
      double cos = dot / mul;
      cos = (cos > 0.999) ? 0.999 : (cos < -0.999) ? -0.999 : cos;
      float temp = (acos(cos) * 180 / M_PI) / 360.0;
      alpha.push_back(temp);
    }
    // 左右
    std::vector<float> beta;
    for (unsigned int i = 0; i < 2; i++) {
      float dot = dist[i].x * dist[i + 2].x + dist[i].y * dist[i + 2].y;
      float mul = sqrt(pow(dist[i].x, 2) + pow(dist[i].y, 2)) *
                  sqrt(pow(dist[i + 2].x, 2) + pow(dist[i + 2].y, 2));
      double cos = dot / mul;
      cos = (cos > 0.999) ? 0.999 : (cos < -0.999) ? -0.999 : cos;
      float temp = (acos(cos) * 180 / M_PI) / 360.0;
      beta.push_back(temp);
    }
    /**/
    float distLR =
        fabs(beta[0] - beta[1]);  // 理论上相等，不相等，则说明左右有偏转
    float distUp =
        alpha[0] - (beta[0] > beta[1]
                        ? beta[0]
                        : beta[1]);  // 理论上不超过0,若超过说明向上仰头
    float distUD = alpha[1] - alpha[0];  // 理论上小于0,若大于，则说明低头了
    float lRThreshold = 0.25;  // 左右偏转,越大，越宽松 0.18  0.25
    float upThreshold = 0.2;   // 向上抬头，越大，越宽松 -0.01  0.2
    float doThreshoold = 0.2;  // 向下低头，越大，越宽松 -0.04 0.2
    if ((distLR > lRThreshold) || (distUp > upThreshold) ||
        (distUD > doThreshoold)) {
      outRange = false;
      return 0;
    }
    // 计算得分
    float scoreLR = 1 - distLR / lRThreshold;  // 左右得分
    // 上下：0.9 1.4 2
    //        float sUD = alpha[0]/alpha[1];
    //        float scoreUD =0;
    //        if (sUD >1.4)
    //            scoreUD = (sUD-2)/(1.4 -2);
    //        else
    //            scoreUD = (sUD-0.9)/(1.4-0.9);
    // 方法2
    float sUD = alpha[0] / alpha[1] / 1.23;
    sUD = sUD < 1 ? sUD : 1.0 / sUD;
    float udScale = 0.4;  // 0.72 0.4
    if (sUD < udScale) {
      outRange = false;
      return 0;
    }
    float scoreUD = (sUD - udScale) / (1 - udScale);
    float score = (scoreLR + scoreUD) / 2;
    outRange = true;
    return score;
  }
};
}  // namespace tracker_sort
}  // namespace algorithm
}  // namespace sophon_stream
