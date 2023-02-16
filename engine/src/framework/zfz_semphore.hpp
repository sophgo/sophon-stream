/*********************************************************** 
* Date: 2016-06-23 
* 
* Author: nick 
* 
* Email: nick1115@163.com 
* 
* Module: 信号量 
* 
* Brief: 模拟windows的semphore,但不具备跨进程能力 
* 
* Note: 
* 
* CodePage: UTF-8 
************************************************************/ 
#ifndef __ZFZ_SEMPHORE_HPP_BY_MOUYUN_2016_06_23__
#define __ZFZ_SEMPHORE_HPP_BY_MOUYUN_2016_06_23__

#include <chrono>
#include <mutex>
#include <condition_variable>

namespace zfz
{

enum 
{
    ZFZ_SEMPHORE_FAIL = (-1),
    ZFZ_SEMPHORE_SUCCESS = 0,
    ZFZ_SEMPHORE_TIME_OUT = 1
};

class Semphore
{
public:
    Semphore(const int init_signals = 0) : signals_(init_signals), blocked_(0) {}
    ~Semphore() {}

private:
    Semphore(const Semphore&) = delete;
    Semphore(Semphore&&) = delete;
    Semphore operator=(const Semphore&) = delete;

public:
    // time_out_ms为负数时表示无限等待 
    int wait(const int time_out_ms = (-1))
    {
        std::unique_lock<std::mutex> guard(lock_);
        if (signals_ > 0)
        {
            --signals_;
            return ZFZ_SEMPHORE_SUCCESS;
        }
        else
        {
            if (time_out_ms == 0)
            {
                return ZFZ_SEMPHORE_TIME_OUT;
            }
            else
            {
                ++blocked_;
            }
        }

        if (time_out_ms >= 0)
        {
            std::chrono::milliseconds wait_time_ms(time_out_ms);
            auto result = cv_.wait_for(guard, wait_time_ms, [&]{ return signals_ > 0; });
            --blocked_;
            if (result)
            {
                --signals_;
                return ZFZ_SEMPHORE_SUCCESS;
            }
            else
            {
                return ZFZ_SEMPHORE_TIME_OUT;
            }
        }
        else
        {
            cv_.wait(guard, [&]{ return signals_ > 0; });
            --blocked_;
            --signals_;
            return ZFZ_SEMPHORE_SUCCESS;
        }
    }

    inline void signal(const int count = 1)
    {
        if (count <= 0) // protect signal not become to release
        {
            return;
        }

        std::lock_guard<std::mutex> guard(lock_);
        signals_ += count;
        if (blocked_ > 0)
        {
            int notify_count = blocked_ < count ? blocked_ : count;
            for (int i = 0; i < notify_count; ++i)
            {
                cv_.notify_one();
            }
        }
    }

    inline void release(const int count)
    {
        if (count <= 0) // protect release not become to signal
        {
            return;
        }

        lock_.lock();
        signals_ -= count;
        if (signals_ < 0)
        {
            signals_ = 0;
        }
        lock_.unlock();
    }

    inline void release_to(int signals)
    {
        if (signals < 0)
        {
            signals = 0;
        }
        lock_.lock();
        signals_ = signals;
        lock_.unlock();
    }

    inline void reset()
    {
        lock_.lock();
        signals_ = 0;
        lock_.unlock();
    }
    
private:
    std::mutex lock_;
    std::condition_variable cv_;
    int signals_ = 0;
    int blocked_ = 0;
}; // class Semphore

} // namespace zfz

#endif