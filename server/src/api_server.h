#pragma once
#include "store.h"
#include <string>
#include <thread>
#include <atomic>

// ─────────────────────────────────────────────────────
//  FluxDB :: APIServer
//
//  REST HTTP API for the web dashboard.
//  Uses cpp-httplib (header-only).
//
//  Endpoints:
//    GET  /api/stats         → JSON: server statistics
//    GET  /api/keys          → JSON: array of all keys
//    POST /api/query         → JSON: { "cmd": "GET foo" } → result
//    GET  /api/health        → 200 OK (for health checks)
// ─────────────────────────────────────────────────────

class APIServer {
public:
    APIServer(KVStore& store, int port = 8080);
    ~APIServer();

    void start();   // blocks
    void stop();

private:
    KVStore&          store_;
    int               port_;
    std::atomic<bool> running_{false};

    // httplib server lives on heap (forward-declared)
    struct Impl;
    Impl* impl_ = nullptr;

    std::string stats_json();
    std::string keys_json();
    std::string execute_command(const std::string& body);
};
