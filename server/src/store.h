#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <optional>
#include <vector>
#include <mutex>
#include <atomic>

class KVStore {
public:
    explicit KVStore(size_t max_size = 1000);
    void        set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool        del(const std::string& key);
    bool        exists(const std::string& key);
    std::vector<std::string> keys();
    size_t      size();
    void        flushall();
    struct Stats { size_t key_count, hits, misses, evictions; double hit_rate; size_t max_size; };
    Stats stats();
private:
    struct Entry { std::string value; std::list<std::string>::iterator lru_it; };
    size_t max_size_;
    std::unordered_map<std::string, Entry> store_;
    std::list<std::string> lru_list_;
    std::mutex mutex_;
    std::atomic<size_t> hits_{0}, misses_{0}, evictions_{0};
    void evict_lru_locked();
    void touch_locked(const std::string& key, Entry& e);
};
