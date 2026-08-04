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
    if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr))<0) throw std::runtime_error("bind() failed");
    if (listen(server_fd_, 128)<0) throw std::runtime_error("listen() failed");
    running_=true;
    std::cout<<"[FluxDB] TCP server listening on port "<<port_<<std::endl;
    start_cleanup_thread();
    while (running_) {
        sockaddr_in ca{}; socklen_t cl=sizeof(ca);
        int cfd=accept(server_fd_,(sockaddr*)&ca,&cl);
        if (cfd<0) continue;
        std::string ip=inet_ntoa(ca.sin_addr);
        std::cout<<"[FluxDB] Client connected: "<<ip<<std::endl;
        std::thread([this,cfd,ip](){handle_client(cfd,ip);}).detach();
    }
}

void TCPServer::handle_client(int fd, std::string ip) {
    std::string banner="+FluxDB v1.0 ready\r\n";
    send(fd,banner.c_str(),banner.size(),0);
    char buf[4096]; std::string partial;
    while(true){
        ssize_t n=recv(fd,buf,sizeof(buf)-1,0);
        if(n<=0)break;
        buf[n]='\0'; partial+=buf;
        size_t pos;
        while((pos=partial.find('\n'))!=std::string::npos){
            std::string line=partial.substr(0,pos); partial=partial.substr(pos+1);
            if(!line.empty()&&line.back()=='\r')line.pop_back();
            if(line.empty())continue;
            std::string resp=dispatch(line);
            send(fd,resp.c_str(),resp.size(),0);
            if(line=="QUIT"||line=="quit"){close(fd);return;}
        }
    }
    close(fd);
    std::cout<<"[FluxDB] Client disconnected: "<<ip<<std::endl;
}

std::string TCPServer::dispatch(const std::string& line){
    Command cmd=Protocol::parse(line);
    if(!cmd.error.empty())return Protocol::error(cmd.error);
    switch(cmd.type){
        case CommandType::PING:    return Protocol::simple("PONG");
        case CommandType::QUIT:    return Protocol::simple("BYE");
        case CommandType::SET:     store_.set(cmd.args[0],cmd.args[1],cmd.ttl_seconds);return Protocol::simple("OK");
        case CommandType::GET:     {auto v=store_.get(cmd.args[0]);return v?Protocol::val(*v):Protocol::null_bulk();}
        case CommandType::DEL:     return Protocol::integer(store_.del(cmd.args[0])?1:0);
        case CommandType::EXISTS:  return Protocol::integer(store_.exists(cmd.args[0])?1:0);
        case CommandType::TTL:     return Protocol::integer(store_.ttl(cmd.args[0]));
        case CommandType::KEYS:    return Protocol::array(store_.keys());
        case CommandType::FLUSHALL:store_.flushall();return Protocol::simple("OK");
        case CommandType::STATS:   {auto s=store_.stats();std::ostringstream o;o<<"keys="<<s.key_count<<" hits="<<s.hits<<" hr="<<s.hit_rate;return Protocol::simple(o.str());}
        default:                   return Protocol::error("Unknown command");
    }
}

void TCPServer::start_cleanup_thread(){
    cleanup_thread_=std::thread([this](){
        while(running_){std::this_thread::sleep_for(std::chrono::seconds(1));store_.cleanup_expired();}
    });
}

void TCPServer::stop(){running_=false;if(server_fd_>=0){close(server_fd_);server_fd_=-1;}if(cleanup_thread_.joinable())cleanup_thread_.join();}
