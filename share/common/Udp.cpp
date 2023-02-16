#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "common/Udp.h"
#include <iostream>

Udp::Udp() {

}

Udp::~Udp() {

}

int Udp::createSocket() {
    mSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (mSockFd == -1)
        return -1;
    return 0;
}

int Udp::setServer(const std::string &address, int port) {
    mAddress = address;
    mPort = port;
    server = std::make_shared<sockaddr_in>();
    struct hostent *he;
    he = gethostbyname(address.c_str());
    if (he == nullptr) {
        std::cout << "gethostbyname() error" << std::endl;
        return -1;
    }
    server->sin_family = AF_INET;
    server->sin_port = htons(port);
    server->sin_addr = *( (struct in_addr *)he->h_addr);
    return 0;
}

int Udp::sendTo(const std::string &msg) {
    int ret = sendto(mSockFd, msg.c_str(), msg.length(), 0, (struct sockaddr *)server.get(), sizeof(*server.get()));
    if (ret == -1) {
        std::cout << "sendTo error" << "ip:" << mAddress << "port:" << mPort << " error:" << errno <<
                  "errorInfo:" << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int Udp::destorySocket() {
    return close(mSockFd);
}
