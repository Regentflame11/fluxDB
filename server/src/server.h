#pragma once
#include "store.h"
#include <string>
#include <atomic>
#include <thread>
#include <functional>

// ─────────────────────────────────────────────────────
//  FluxDB :: TCPServer
//
//  Listens on a port, accepts client connections,
//  reads commands line by line, dispatches to KVStore.
//  Each client runs in its own thread.
// ─────────────────────────────────────────────────────
class TCPServer {
public:
    TCPServer(KVStore& store, int port = 6379);
    ~TCPServer();

    // Start listening (blocks until stop() is called)
    void start();

    // Signal the server to stop
    void stop();

private:
    KVStore&          store_;
    int               port_;
    int               server_fd_ = -1;
    std::atomic<bool> running_{false};

    // Cleanup thread for expired keys (runs every 1 second)
    std::thread cleanup_thread_;

    void handle_client(int client_fd, std::string client_addr);
    std::string dispatch(const std::string& line);
    void start_cleanup_thread();
};
