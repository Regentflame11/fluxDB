#include "store.h"
KVStore::KVStore(size_t max_size) : max_size_(max_size) {}

void KVStore::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it != store_.end()) { lru_list_.erase(it->second.lru_it); store_.erase(it); }
    lru_list_.push_front(key);
    store_[key] = { value, lru_list_.begin() };
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) { ++misses_; return std::nullopt; }
    ++hits_;
    return it->second.value;
}

void KVStore::touch_locked(const std::string& key, Entry& e) {
    lru_list_.erase(e.lru_it); lru_list_.push_front(key); e.lru_it = lru_list_.begin();
}
