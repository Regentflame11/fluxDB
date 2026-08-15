# FluxDB

Fast, dynamic in-memory Key-Value store built in C++ with a real-time web dashboard, Docker deployment, and automated CI/CD to AWS EC2.

## Architecture

```
GitHub Push → GitHub Actions CI/CD
  1. Build C++ server (cmake)
  2. Run smoke tests (PING, SET, GET, TTL)
  3. Build Docker images
  4. SSH deploy to AWS EC2

EC2 Instance (24/7):
  fluxdb-server  → TCP :6379 | REST :8080
  fluxdb-dash    → nginx :3000
```

## Commands

| Command | Description |
|---|---|
| `PING` | Health check, returns PONG |
| `SET key value` | Store a value |
| `SET key value EX 60` | Store with 60s TTL |
| `GET key` | Retrieve a value |
| `DEL key` | Delete a key |
| `EXISTS key` | Check if key exists |
| `TTL key` | Seconds until expiry |
| `KEYS` | List all keys |
| `STATS` | Hit rate, evictions, key count |
| `FLUSHALL` | Delete all keys |

## Quick Start

```bash
# Docker Compose
docker compose up --build
# Dashboard: http://localhost:3000

# Build from source
cd server && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/fluxdb --tcp-port 6379 --api-port 8080
```

## Tech Stack

- **C++17** — core server, data structures
- **cpp-httplib** — REST API (header-only, zero deps)
- **Docker** + **nginx** — containerized deployment
- **GitHub Actions** — CI/CD pipeline
- **AWS EC2** — t2.micro free tier hosting
- **Bash** — deployment automation

## Data Structures

| Structure | Use |
|---|---|
| `unordered_map` | O(1) key lookup |
| Doubly Linked List | LRU eviction order |
| `steady_clock` | TTL timestamps |
| `std::thread` | Per-client TCP handling |
| `std::atomic` | Lock-free stats counters |
