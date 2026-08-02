#include "store.h"
KVStore::KVStore(size_t max_size) : max_size_(max_size) {}

void KVStore::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it != store_.end()) {
        lru_list_.erase(it->second.lru_it);
        store_.erase(it);
    } else if (store_.size() >= max_size_) {
        evict_lru_locked(); // evict least-recently-used when at capacity
    }
    lru_list_.push_front(key);
    store_[key] = { value, lru_list_.begin() };
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) { ++misses_; return std::nullopt; }
    touch_locked(key, it->second);
    ++hits_;
    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    lru_list_.erase(it->second.lru_it);
    store_.erase(it);
    return true;
}

bool KVStore::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.count(key) > 0;
}

std::vector<std::string> KVStore::keys() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> r;
    for (auto& [k, v] : store_) r.push_back(k);
    return r;
}

size_t KVStore::size() { std::lock_guard<std::mutex> l(mutex_); return store_.size(); }

void KVStore::flushall() {
    std::lock_guard<std::mutex> lock(mutex_);
    store_.clear(); lru_list_.clear();
}

KVStore::Stats KVStore::stats() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t h = hits_, m = misses_;
    return { store_.size(), h, m, evictions_, (h+m>0)?(double)h/(h+m)*100:0, max_size_ };
}

void KVStore::evict_lru_locked() {
    if (lru_list_.empty()) return;
    store_.erase(lru_list_.back()); lru_list_.pop_back(); ++evictions_;
}

void KVStore::touch_locked(const std::string& key, Entry& e) {
    lru_list_.erase(e.lru_it); lru_list_.push_front(key); e.lru_it = lru_list_.begin();
}
