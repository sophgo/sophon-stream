#pragma once

#include "framework/ElementNew.h"
//#include "stream/DecoderApi.h"
#include "stream/MultiMediaApi.h"
#include "common/Graphics.hpp"

namespace sophon_stream {
namespace element {

struct ChannelOperateRequest {
    enum class ChannelOperate {
        START,
        STOP,
        PAUSE,
        RESUME,
    };
    int channelId;
    ChannelOperate operation;
    std::string json;
};

struct ChannelOperateResponse {
    common::ErrorCode errorCode;
    std::string errorInfo;
};

struct ChannelTask {
    ChannelOperateRequest request;
    ChannelOperateResponse response;
};

class ThreadWrapper {
    friend class DecoderElement;

    using InitHandler = std::function<common::ErrorCode(void)>;
    using DataHandler = std::function<common::ErrorCode(void)>;
    using UninitHandler = std::function<common::ErrorCode(void)>;

    enum class ThreadStatus {
        RUN,
        STOP,
        PAUSE,
        RESUME,
    };

  public:
    ThreadWrapper() {}
    ~ThreadWrapper() {}
    void init(InitHandler initHandler, DataHandler dataHandler, UninitHandler uninitHandler) {
        mInitHandler = initHandler;
        mDataHandler = dataHandler;
        mUninitHandler = uninitHandler;
    }
    common::ErrorCode start() {
        IVS_INFO("Start element thread start, element id: {0:d}");

        if (ThreadStatus::STOP != mThreadStatus) {
            IVS_ERROR("Can not start, current thread status is not stop");
            return common::ErrorCode::THREAD_STATUS_ERROR;
        }

        mThreadStatus = ThreadStatus::RUN;

        mSpThread = std::make_shared<std::thread>(std::bind(&ThreadWrapper::run, this));

        IVS_INFO("Start element thread finish, element id: {0:d}");
        return common::ErrorCode::SUCCESS;
    }

    common::ErrorCode stop(bool sync = true) {
        IVS_INFO("Stop element thread start, element id: {0:d}");

        if (ThreadStatus::STOP == mThreadStatus) {
            IVS_ERROR("Can not stop, current thread status is stop");
            return common::ErrorCode::THREAD_STATUS_ERROR;
        }

        mThreadStatus = ThreadStatus::STOP;

        (sync)?(mSpThread->join()):(mSpThread->detach());

        mSpThread.reset();

        IVS_INFO("Stop element thread finish, element id: {0:d}");
        return common::ErrorCode::SUCCESS;
    }

    common::ErrorCode pause() {
        IVS_INFO("Pause element thread start, element id: {0:d}");

        if (ThreadStatus::RUN != mThreadStatus) {
            IVS_ERROR("Can not pause, current thread status is not run");
            return common::ErrorCode::THREAD_STATUS_ERROR;
        }

        mThreadStatus = ThreadStatus::PAUSE;

        IVS_INFO("Pause element thread finish, element id: {0:d}");
        return common::ErrorCode::SUCCESS;
    }

    common::ErrorCode resume() {
        IVS_INFO("Resume element thread start, element id: {0:d}");

        if (ThreadStatus::PAUSE != mThreadStatus) {
            IVS_ERROR("Can not resume, current thread status is not pause");
            return common::ErrorCode::THREAD_STATUS_ERROR;
        }

        mThreadStatus = ThreadStatus::RUN;

        IVS_INFO("Resume element thread finish, element id: {0:d}");
        return common::ErrorCode::SUCCESS;
    }
  private:
    ThreadWrapper(const ThreadWrapper&) = delete;
    ThreadWrapper& operator=(const ThreadWrapper&) = delete;
    void run() {
        common::ErrorCode ret = mInitHandler();
        while (ThreadStatus::STOP != mThreadStatus&&ret==common::ErrorCode::SUCCESS) {
            mDataHandler();
        }
        if(ret!=common::ErrorCode::SUCCESS){
            mUninitHandler();
        }
      
    }
    std::shared_ptr<std::thread> mSpThread = nullptr;
    ThreadStatus mThreadStatus = ThreadStatus::STOP;
    InitHandler mInitHandler{nullptr};
    DataHandler mDataHandler{nullptr};
    UninitHandler mUninitHandler{nullptr};
};

struct ChannelInfo {
    int mFrameCount = 0;
    std::shared_ptr<multimedia::MultiMediaApi> mSpDecoder;
    std::shared_ptr<std::mutex> mMtx;
    std::shared_ptr<std::condition_variable> mCv;
    std::shared_ptr<ThreadWrapper> mThreadWrapper;
};

class DecoderElement : public framework::Element {
  public:
    DecoderElement();
    ~DecoderElement() override;

    DecoderElement(const DecoderElement&) = delete;
    DecoderElement& operator =(const DecoderElement&) = delete;
    DecoderElement(DecoderElement&&) = default;
    DecoderElement& operator =(DecoderElement&&) = default;

    static constexpr const char* X = "x";
    static constexpr const char* Y = "y";
    static constexpr const char* W = "w";
    static constexpr const char* H = "h";
    static constexpr const char* NAME = "decoder_element";
    static constexpr const char* ROI = "roi";
    static constexpr const char* JSON_SHARED_OBJECT_FIELD = "shared_object";

    static constexpr const char* JSON_URL = "url";
    static constexpr const char* JSON_TIMEOUT = "timeout";
    static constexpr const char* JSON_REOPEN_TIMES = "reopen_times";
    static constexpr const char* JSON_SOURCE_TYPE = "source_type";
    static constexpr const char* JSON_SKIP_COUNT = "skip_count";
    static constexpr const char* JSON_BATCH_SIZE = "batch_size";

    static void doSth();

  private:

    common::ErrorCode initInternal(const std::string& json) override;
    void uninitInternal() override;
    void onStart()override;
    void onStop()override;
    common::ErrorCode doWork() override;

    common::ErrorCode startTask(std::shared_ptr<ChannelTask>& channelTask);

    common::ErrorCode stopTask(std::shared_ptr<ChannelTask>& channelTask);

    common::ErrorCode pauseTask(std::shared_ptr<ChannelTask>& channelTask);

    common::ErrorCode resumeTask(std::shared_ptr<ChannelTask>& channelTask);

    common::ErrorCode parseJson(const std::string& json, std::string& url, int& sourceType, int& reopentimes);

    common::ErrorCode process(const bool lastFrame,const int sourceType,const int reopentimes,const int capacity,
            const std::shared_ptr<ChannelTask>& channelTask, 
            const std::shared_ptr<ChannelInfo>& channelInfo);

    common::ErrorCode reopen(const int reopentimes, const std::shared_ptr<ChannelTask>& channelTask, const std::shared_ptr<ChannelInfo>& channelInfo);
  private:
    int mSkipCount = 0;
    int mBatchSize = 0;
    std::shared_ptr<void> mSharedObjectHandle;
    std::map<int,std::shared_ptr<ChannelInfo>> mThreadsPool;
    std::mutex mThreadsPoolMtx;
};

} // namespace element
} // namespace sophon_stream

