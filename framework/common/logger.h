#pragma once

#include <iostream>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

class Logger {
 private:
  std::shared_ptr<spdlog::logger> mLogger = nullptr;

 public:
  Logger(const std::string& path = nullptr);
  ~Logger();

  /**
   * 获取一个实例
   *
   * @return 返回一个实例
   */
  std::shared_ptr<spdlog::logger>& getInstance();

  static std::shared_ptr<spdlog::logger>& getLogger(
      const std::string& path = "");

 private:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
};

void logInit(const std::string& name = "info", const std::string& path = "");

#define IVSLOG_STR(integer) #integer
#define IVSLOG_STR_HELP(integer) IVSLOG_STR(integer)
#ifdef __FILENAME__
#define IVSLOG_FILE __FILENAME__
#else
#define IVSLOG_FILE __FILE__
#endif

#define IVSLOG_FMT(fmt) "[" IVSLOG_FILE ":" IVSLOG_STR_HELP(__LINE__) "] " fmt

template <typename... Args>
inline void trace(const char* fmt, const Args&... args) {
  return Logger::getLogger()->trace(fmt, args...);
}

#define IVS_TRACE(fmt, ...) ::trace(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template <typename... Args>
inline void debug(const char* fmt, const Args&... args) {
  return Logger::getLogger()->debug(fmt, args...);
}
#define IVS_DEBUG(fmt, ...) ::debug(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template <typename... Args>
inline void info(const char* fmt, const Args&... args) {
  return Logger::getLogger()->info(fmt, args...);
}
#define IVS_INFO(fmt, ...) ::info(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template <typename... Args>
inline void warn(const char* fmt, const Args&... args) {
  return Logger::getLogger()->warn(fmt, args...);
}
#define IVS_WARN(fmt, ...) ::warn(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template <typename... Args>
inline void error(const char* fmt, const Args&... args) {
  return Logger::getLogger()->error(fmt, args...);
}
#define IVS_ERROR(fmt, ...) ::error(IVSLOG_FMT(fmt), ##__VA_ARGS__)

template <typename... Args>
inline void critical(const char* fmt, const Args&... args) {
  return Logger::getLogger()->critical(fmt, args...);
}
#define IVS_CRITICAL(fmt, ...) ::critical(IVSLOG_FMT(fmt), ##__VA_ARGS__)
