#pragma once

#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/null_mutex.h"
#include <mutex>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "common/Udp.h"

#include <nlohmann/json.hpp>

using boost::asio::ip::udp;
using boost::asio::ip::address;

#define DEST_PORT 12202
#define DEST_IP_ADDRESS  "127.0.0.1"

namespace spdlog {
namespace sinks {
template<typename Mutex>
class udp_sink : public spdlog::sinks::base_sink<Mutex> {
  public:
    explicit udp_sink(const std::string &address, const int port) {
        mUdp.createSocket();
        mUdp.setServer(address, port);
    }
    ~udp_sink() {
        mUdp.destorySocket();
    }

    void reconnect(const std::string &address, const int port) {
        mUdp.setServer(address, port);
    }

  protected:

    /* 将传入的msg转换成json格式，通过udp发送到udp server
     * 输入参数：msg: 其原始格式为
     *      [_service][engine][level][info][file][../src/main.cpp:138] Log level set to info//taskId=1,type=2
     * 转换后的json格式为:
     *      {
     *         "_service": "engine",
     *         "level": 6,
     *         "file":"..\/src\/main.cpp:138",
     *         "short_message": " Log level set to info",
     *         "taskId": "1",
     *         "type": "2"
     *      }
     **/
    void sink_it_(const spdlog::details::log_msg &msg) override {

        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.raw contains pre formatted log
        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:

        fmt::memory_buffer formatted;
        sink::formatter_->format(msg, formatted);

        std::string buffer = fmt::to_string(formatted);
        std::string str(buffer.c_str(), buffer.size() - 1);

        std::string error;

        nlohmann::json j;

        // generate json from log
        std::vector<std::string> result;
        splitString(result, str, '[', ']', error);
        if (error != "") {
            std::cout << error << std::endl;
            return;
        }
        for (int i = 0; i < result.size() - 2; i += 2) {
            j[result[i]] = result[i + 1];
        }

        std::vector<std::string> content;
        splitString(content, result[result.size() - 1], "//#", error);
        if (error != "") {
            std::cout << error << std::endl;
            return;
        }

        j["short_message"] = content[0];
        if (content.size() > 1) {
            std::vector<std::string> tags;
            boost::split(tags, content[content.size() - 1], boost::is_any_of("=,"), boost::token_compress_on);
            for (int i = 0; i < tags.size(); i += 2) {
                j[tags[i]] = tags[i + 1];
            }
        }

        j["level"] = to_graylog_level[msg.level];

        std::string json = j.dump();

        // send json to log udpserver
        mUdp.sendTo(json);

    }

    void flush_() override {}

  private:
    Udp mUdp;

    /*
     *      trace = 0,     //8
            debug = 1,     //7
            info = 2,      //6
            warn = 3,      //4
            err = 4,       //3
            critical = 5,  //2
            off = 6
    */
    int to_graylog_level[6] = {8, 7, 6, 4, 3, 2};

    std::size_t splitString(std::vector<std::string> &results, const std::string &src, const std::string &symbol, std::string &error) {
        std::size_t pos = src.find(symbol);
        std::string strCut = src;
        while (pos != std::string::npos)

            try {
                results.push_back(strCut.substr(0, pos));
                strCut = strCut.substr(pos + symbol.length());
                pos = strCut.find(symbol);
                if (pos == std::string::npos) {
                    results.push_back(strCut);
                }

            } catch (std::exception &e) {
                error = "splitString failed! src:" + src + " error:" + e.what();
            }

        if (results.size() == 0) {
            results.push_back(src);
        }

        return pos;
    }

    void splitString(std::vector<std::string> &results, const std::string &src,
                     const char symbol0, const char symbol1, std::string &error) {
        std::size_t pos0 = src.find(symbol0);
        std::size_t pos1 = src.find(symbol1);
        std::string strCut = src;
        while ((pos0 != std::string::npos) && (pos1 != std::string::npos))
            try {
                results.push_back(strCut.substr(pos0 + 1, pos1 - pos0 - 1));
                strCut = strCut.substr(pos1 + 1);
                pos0 = strCut.find(symbol0);
                pos1 = strCut.find(symbol1);
                if (pos0 != 0 && strCut != "") {
                    results.push_back(strCut);
                    break;
                }
            } catch (std::exception &e) {
                error = "splitString failed! src:" + src + " error:" + e.what();
            }
    }

};

using udp_sink_mt = udp_sink<std::mutex>;
using udp_sink_st = udp_sink<spdlog::details::null_mutex>;
}
}

