/// @file lru.cc
/// @brief Implementation of the least recently used replacement policy

#include "lru.h"

ADD_REPLACER_TO_CMD_LINE(LRU);

LRU::LRU(CacheABC& cache, uint32_t num_sets, uint32_t assoc)
    : ReplacementPolicy(cache, num_sets, assoc) {
    age = new uint32_t * [num_sets];
    for (uint32_t i = 0; i < num_sets; i++)
        age[i] = new uint32_t[assoc]{};
}
LRU::~LRU() {
    for (uint32_t i = 0; i < num_sets; i++)
        delete[] age[i];
    delete[] age;
}

uint32_t LRU::getVictim(uint32_t set_idx) {
    uint32_t* set = age[set_idx];
    uint32_t max_idx = 0, max = 0;
    for (uint32_t i = 0; i < assoc; i++) {
        if (!cache.getLineState(set_idx, i)) return i;
        if (set[i] > max) {
            max = set[i];
            max_idx = i;
        }
    }
    return max_idx;
}

void LRU::touch(uint32_t set_idx, uint32_t way_idx) {
    uint32_t* set = age[set_idx];
    uint32_t line_age = set[way_idx];
    for (uint32_t i = 0; i < assoc; i++)
        if (set[i] <= line_age) set[i]++;
    set[way_idx] = 0;
}

void LRU::printState(uint32_t set_idx) {
    if (set_idx >= num_sets) return;
    uint32_t* set = age[set_idx];
    std::cout << set[0];
    for (uint32_t i = 1; i < assoc; i++)
        std::cout << ' ' << set[i];
}
