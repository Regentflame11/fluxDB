#include "server.h"
#include "store.h"
#include <iostream>
int main(){
    KVStore store(1000);
    TCPServer tcp(store,6379);
    std::cout<<"[FluxDB] Starting..."<<std::endl;
    tcp.start();
}
