#include "common/Logger.h"
#include <unistd.h>
#include "common/Udp.h"
const char *LoggerName = "engine";

Logger::Logger(const std::string &path, const std::string &url) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%H:%M:%S %z] [engine] [%^%l%$] %v");

    std::vector<spdlog::sink_ptr> sinks {console_sink};

    // Add file sink
    if (!path.empty()) {
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, 1024 * 1024 * 10, 100);
        sinks.push_back(file_sink);
    }

    // TODO: add log server sink
    std::string address;
    int port = 0;
    analyseUrl(url, address, port);
    auto udp_sink = std::make_shared<spdlog::sinks::udp_sink_mt>(address, port);
    udp_sink->set_pattern("[_service][%n][file]%v");
    sinks.push_back(udp_sink);

    // Use sinks to create logger instance
    auto logger = std::make_shared<spdlog::logger>(LoggerName, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::info);

    mLogger = logger;
}

Logger::~Logger() {
    mLogger.reset();
}

std::shared_ptr<spdlog::logger> &Logger::getInstance() {
    return mLogger;
}

void Logger::changeServer(const std::string &udpAddress) {
    std::string address;
    int port = 0;
    analyseUrl(udpAddress, address, port);
    auto logger = Logger::getLogger();
    for (auto &sinkSptr : logger->sinks()) {
        spdlog::sinks::udp_sink_mt *udpSinkPtr =
            dynamic_cast<spdlog::sinks::udp_sink_mt *>(sinkSptr.get());
        if (nullptr != udpSinkPtr) {
            std::cout << "std cout info changeUdpAddress address:" << address <<
                      " port:" << port << std::endl;
            udpSinkPtr->reconnect(address, port);
        }
    }
}

std::shared_ptr<spdlog::logger> &Logger::getLogger(const std::string &name, const std::string &path, const std::string &url) {
    static Logger logger(path, url);
    return logger.getInstance();
}

std::size_t Logger::analyseUrl(const std::string &url, std::string &address, int &port) {
    size_t pos = url.find(':');
    try {
        address = url.substr(0, pos);
        port = std::stoi(url.substr(pos + 1));
    } catch (std::exception &e) {
        std::cout << "url: " << url << " error: " << e.what() << std::endl;
    }

    return pos;
}

void logInit(const std::string &name, const std::string &path, const std::string &url) {
    auto logger = Logger::getLogger(name, path, url);
    logger->set_level(spdlog::level::from_str(name));
};

