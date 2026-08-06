#include "server.h"
#include "api_server.h"
#include "store.h"
#include <iostream>
#include <thread>
#include <csignal>
#include <memory>
static std::unique_ptr<TCPServer> g_tcp;
static std::unique_ptr<APIServer> g_api;
void handle_signal(int){
    std::cout<<"\n[FluxDB] Shutting down..."<<std::endl;
    if(g_tcp)g_tcp->stop();
    if(g_api)g_api->stop();
    exit(0);
}
int main(int argc,char* argv[]){
    int tcp_port=6379,api_port=8080,max_keys=1000;
    for(int i=1;i<argc-1;++i){std::string a=argv[i];if(a=="--tcp-port")tcp_port=std::stoi(argv[i+1]);if(a=="--api-port")api_port=std::stoi(argv[i+1]);if(a=="--max-keys")max_keys=std::stoi(argv[i+1]);}
    std::cout<<"[FluxDB] v1.0.0 | tcp="<<tcp_port<<" api="<<api_port<<" max_keys="<<max_keys<<std::endl;
    std::signal(SIGINT,handle_signal);
    std::signal(SIGTERM,handle_signal);
    KVStore store(max_keys);
    g_api=std::make_unique<APIServer>(store,api_port);
    std::thread([&](){g_api->start();}).detach();
    g_tcp=std::make_unique<TCPServer>(store,tcp_port);
    g_tcp->start();
}
