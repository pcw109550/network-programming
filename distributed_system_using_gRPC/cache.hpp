#ifndef __CACHE_HPP_
#define __CACHE_HPP_
#include <iostream>
#include <string>
#include <unordered_map>
#include <assert.h>

class Cache {
    public:
        Cache(int _total_size) {
            total_size = _total_size;
            current_size = 0;
        }
        
        std::string CheckCache(std::string &key) {
            if (cache_map.find(key) == cache_map.end()) {
                // std::cout << "Cache miss: " << key << std::endl;
                return "\0";
            }
            // std::cout << "Cache hit: " << key << " -> " << cache_map[key] << std::endl;
            return cache_map[key];
        }

        void update(std::string key, std::string value) {
            int acc_size = key.size() + value.size();
            if (acc_size > total_size) {
                // cannot insert to cache since too big
                return;
            }
            while (current_size + acc_size > total_size && !cache_map.empty()) {
                auto it = cache_map.begin();
                if (it->first == key) {
                    // randomly picked key cand for eviction was same as
                    // newly added key, so do nothing
                    return;
                }
                int deleted_size = it->first.size() + it->second.size(); 
                cache_map.erase(it);
                current_size -= deleted_size;
                assert(current_size >= 0);
            }
            // add new mapping to cache
            cache_map[key] = value;
            current_size += acc_size;
            // assert (current_size <= total_size);
            if (current_size > total_size) {
                // BUG! must be never reached
                current_size = 0;
                cache_map.clear();
                std::cout << cache_map.size() << ' ' << current_size << std::endl;
            }
            //std::cout << "[" << current_size << " / " << total_size << "]" << std::endl;
        } 

        void Cachestate(void) {
            for (auto it : cache_map)
                std::cout << it.first << " -> " << it.second << std::endl;
            std::cout << "total mapping num: " << cache_map.size() << ' ' << current_size << std::endl;
        }

    private:
        int total_size = 0;
        int current_size = 0;
        std::unordered_map<std::string, std::string> cache_map;
};

#endif