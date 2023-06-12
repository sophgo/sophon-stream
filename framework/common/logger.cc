#include "common/logger.h"

#include <unistd.h>
const char* LoggerName = "engine";

Logger::Logger(const std::string& path) {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern("[%H:%M:%S %z] [engine] [%^%l%$] %v");

  std::vector<spdlog::sink_ptr> sinks{console_sink};

  // Add file sink
  if (!path.empty()) {
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        path, 1024 * 1024 * 10, 100);
    sinks.push_back(file_sink);
  }

  // Use sinks to create logger instance
  auto logger =
      std::make_shared<spdlog::logger>(LoggerName, sinks.begin(), sinks.end());
  logger->set_level(spdlog::level::info);

  mLogger = logger;
}

Logger::~Logger() { mLogger.reset(); }

std::shared_ptr<spdlog::logger>& Logger::getInstance() { return mLogger; }

std::shared_ptr<spdlog::logger>& Logger::getLogger(const std::string& path) {
  static Logger logger(path);
  return logger.getInstance();
}

void logInit(const std::string& name, const std::string& path) {
  auto logger = Logger::getLogger(path);
  logger->set_level(spdlog::level::from_str(name));
};
