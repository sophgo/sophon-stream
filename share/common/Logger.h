#pragma once
//#define SPDLOG_TRACE_ON
//#define SPDLOG_DEBUG_ON

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <iostream>

// #include "udp_sink.h"

enum LoggerOutput {
    LOG_CONSOLE = 0,//日志在控制台打印
    LOG_FILE = 1//日志在文件打印
};

class Logger {
  private:
    std::shared_ptr<spdlog::logger> mLogger = nullptr;

  public:
    Logger(const std::string &path = nullptr, const std::string &url = nullptr);
    ~Logger();
    /**
     * 创建一个实例
     *
     * @param[in] output       日志output
     * @param[in] fileName    生成的文件名,此文件在程序执行目录下
     */
    void init(
        const LoggerOutput output = LOG_CONSOLE,
        const std::string &fileName = "");
    /**
     * 获取一个实例
     *
     * @return 返回一个实例
     */
    std::shared_ptr<spdlog::logger> &getInstance();

    /**
     * 销毁一个实例
     *
     * @param[in] logger_type  日志类型
     * @param[in] file_name    生成的文件名,此文件在程序执行目录下
     */
    void destroy();

    /**
     * 修改udp日志服务地址 并进行重连
     *
     * @param[in] udpAddress  udp日志服务地址
     * @param[in] port        udp日志服务端口号
     */
    static void changeServer(const std::string &udpAddress);


    static std::shared_ptr<spdlog::logger> &getLogger(const std::string &name = "info", const std::string &path = "", const std::string &url = "");

  private:
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    static std::size_t analyseUrl(const std::string &url , std::string &address, int &port);
};


extern void logInit(const std::string &name = "info", const std::string &path = "", const std::string &url = "");


#define IVSLOG_STR(integer) #integer
#define IVSLOG_STR_HELP(integer) IVSLOG_STR(integer)
#ifdef __FILENAME__
#define IVSLOG_FILE __FILENAME__
#else
#define IVSLOG_FILE __FILE__
#endif

#define IVSLOG_FMT(fmt) \
    "[" IVSLOG_FILE ":" IVSLOG_STR_HELP(__LINE__) "] " fmt

template<typename... Args>
inline void trace(const char *fmt, const Args &... args ) {
    return Logger::getLogger()->trace(fmt, args...);
}

#define IVS_TRACE(fmt, ...) ::trace(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template<typename... Args>
inline void debug(const char *fmt, const Args &... args ) {
    return Logger::getLogger()->debug(fmt, args...);
}
#define IVS_DEBUG(fmt, ...) ::debug(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template<typename... Args>
inline void info(const char *fmt, const Args &... args ) {
    return Logger::getLogger()->info(fmt, args...);
}
#define IVS_INFO(fmt, ...) ::info(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template<typename... Args>
inline void warn(const char *fmt, const Args &... args ) {
    return Logger::getLogger()->warn(fmt, args...);
}
#define IVS_WARN(fmt, ...) ::warn(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template<typename... Args>
inline void error(const char *fmt, const Args &... args ) {
    return Logger::getLogger()->error(fmt, args...);
}
#define IVS_ERROR(fmt, ...) ::error(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template<typename... Args>
inline void critical(const char *fmt, const Args &... args ) {
    return Logger::getLogger()->critical(fmt, args...);
}
#define IVS_CRITICAL(fmt, ...) ::critical(IVSLOG_FMT(fmt), ##__VA_ARGS__)


