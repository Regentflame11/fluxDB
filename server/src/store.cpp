#include "store.h"
KVStore::KVStore(size_t max_size) : max_size_(max_size) {}

void KVStore::set(const std::string& key, const std::string& value, int ttl_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it != store_.end()) { lru_list_.erase(it->second.lru_it); store_.erase(it); }
    else if (store_.size() >= max_size_) { evict_lru_locked(); }
    lru_list_.push_front(key);
    Entry e; e.value = value; e.lru_it = lru_list_.begin();
    e.has_ttl = ttl_seconds > 0;
    if (e.has_ttl) e.expiry = std::chrono::steady_clock::now() + std::chrono::seconds(ttl_seconds);
    store_[key] = std::move(e);
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) { ++misses_; return std::nullopt; }
    if (is_expired_locked(it->second)) { lru_list_.erase(it->second.lru_it); store_.erase(it); ++misses_; return std::nullopt; }
    touch_locked(key, it->second); ++hits_;
    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    lru_list_.erase(it->second.lru_it); store_.erase(it); return true;
}

bool KVStore::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    if (is_expired_locked(it->second)) { lru_list_.erase(it->second.lru_it); store_.erase(it); return false; }
    return true;
}

int KVStore::ttl(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(key);
    if (it == store_.end()) return -2;
    if (!it->second.has_ttl) return -1;
    if (is_expired_locked(it->second)) { lru_list_.erase(it->second.lru_it); store_.erase(it); return -2; }
    return (int)std::chrono::duration_cast<std::chrono::seconds>(it->second.expiry - std::chrono::steady_clock::now()).count();
}

std::vector<std::string> KVStore::keys() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> r;
    for (auto& [k, v] : store_) if (!v.has_ttl || v.expiry > now) r.push_back(k);
    return r;
}

size_t KVStore::size() { std::lock_guard<std::mutex> l(mutex_); return store_.size(); }

void KVStore::flushall() { std::lock_guard<std::mutex> lock(mutex_); store_.clear(); lru_list_.clear(); }

KVStore::Stats KVStore::stats() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t h = hits_, m = misses_;
    return { store_.size(), h, m, evictions_, (h+m>0)?(double)h/(h+m)*100:0, max_size_ };
}

void KVStore::cleanup_expired() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    for (auto it = store_.begin(); it != store_.end(); )
        if (it->second.has_ttl && it->second.expiry <= now) { lru_list_.erase(it->second.lru_it); it = store_.erase(it); }
        else ++it;
}

void KVStore::evict_lru_locked() { if (lru_list_.empty()) return; store_.erase(lru_list_.back()); lru_list_.pop_back(); ++evictions_; }
bool KVStore::is_expired_locked(const Entry& e) const { return e.has_ttl && std::chrono::steady_clock::now() >= e.expiry; }
void KVStore::touch_locked(const std::string& key, Entry& e) { lru_list_.erase(e.lru_it); lru_list_.push_front(key); e.lru_it = lru_list_.begin(); }
