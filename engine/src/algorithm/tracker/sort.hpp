#pragma


#include "TrackerChannel.hpp"


namespace sophon_stream {
namespace algorithm {
namespace tracker_sort {


class TrackerChannels {
  public:
    //单帧的mentdata数据
    using PUT_TASK_FUNC = std::function<void (std::shared_ptr<common::ObjectMetadata>&)>;

    TrackerChannels();
    ~TrackerChannels();

    TrackerChannels(const TrackerChannels&) = delete;
    TrackerChannels& operator =(const TrackerChannels&) = delete;
    TrackerChannels(TrackerChannels&&) = default;
    TrackerChannels& operator =(TrackerChannels&&) = default;

    /**
    * 初始化多路的跟踪器:跟踪器和质量评估模块的初始化参数
    * @param[in]   json      json配置
    * @return: 成功或者失败
    */
    common::ErrorCode init(const std::string& json);

    /**
    * 跟踪器进行跟踪
    * @param[in]   objectMetadata      单帧的mentdata数据
    * @param[in]   fc    回调函数
    */
    void process(std::shared_ptr<common::ObjectMetadata> objectMetadatas, const PUT_TASK_FUNC &fc);
    /**
    * 反初始化函数
    */
    void uninit();

  private:
    /**
    * 清除跟踪器的缓冲
    */
    void clearBuffer();
    /**
    * 删除对应ID的跟踪器
    * @param[in]   taskID      跟踪器的ID
    */
    void removeTracker(const int &taskId);
    int mTopN = 1;
    float mIou = 0.25f;
    int mMaxAge = 1;
    int mUpdateTimes = 0;
    unsigned long long mMinHins = 3;
    long mTimebase = 3000000;
    std::map<int, std::shared_ptr<TrackerChannel>> mSorts;

    std::shared_ptr<QualityControl> pQualityControl=nullptr;

};
}
}
}