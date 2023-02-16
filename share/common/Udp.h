#pragma once

#include<string>
#include<memory>
struct sockaddr_in;

class Udp {

  public:
    Udp();
    ~Udp();
    int createSocket();
    int setServer(const std::string &address, int port);
    int sendTo(const std::string &msg);
    int destorySocket();
  private:
    int mSockFd;
    int mPort;
    std::string mAddress;
    std::shared_ptr<sockaddr_in> server = nullptr;

};

