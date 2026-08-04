#include "server.h"
#include "protocol.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <sstream>

TCPServer::TCPServer(KVStore& store, int port) : store_(store), port_(port) {}
TCPServer::~TCPServer() { stop(); }

void TCPServer::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) throw std::runtime_error("socket() failed");
    int opt=1; setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_addr.s_addr=INADDR_ANY; addr.sin_port=htons(port_);
    if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) throw std::runtime_error("bind() failed");
    if (listen(server_fd_, 128) < 0) throw std::runtime_error("listen() failed");
    running_ = true;
    std::cout << "[FluxDB] TCP server listening on port " << port_ << std::endl;
    while (running_) {
        sockaddr_in ca{}; socklen_t cl=sizeof(ca);
        int cfd = accept(server_fd_, (sockaddr*)&ca, &cl);
        if (cfd < 0) continue;
        // TODO: handle in thread
        close(cfd);
    }
}

void TCPServer::stop() { running_=false; if(server_fd_>=0){close(server_fd_);server_fd_=-1;} if(cleanup_thread_.joinable())cleanup_thread_.join(); }
void TCPServer::start_cleanup_thread() {}
