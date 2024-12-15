/// @file memory_bus.cc
/// @brief Implementation of the MemoryBus class methods

#include <iostream>

#include "cache.h"
#include "memory_bus.h"

MemoryBus::MemoryBus(cache_config& config) : config(config) {
    // Make sure to zero-initialize array of caches
    caches = new Cache * [MAX_N_CACHES] {};
    tag_offset = MSB(config.cache_size / config.assoc);
}
MemoryBus::~MemoryBus() {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        delete caches[i];
    operator delete[](caches);
}

void MemoryBus::issuePrRd(addr_t addr, uint32_t cache_id) {
    // Dynamically allocate cache
    if (!caches[cache_id]) caches[cache_id] = new Cache(*this, flushed, cache_id, config);

    flushed = false;
    caches[cache_id]->receivePrRd(addr);
}

void MemoryBus::issuePrWr(addr_t addr, uint32_t cache_id) {
    // Dynamically allocate cache
    if (!caches[cache_id]) caches[cache_id] = new Cache(*this, flushed, cache_id, config);

    flushed = false;
    caches[cache_id]->receivePrWr(addr);
}

bool MemoryBus::issueBusMsg(bus_msg_e bus_msg, addr_t addr, uint32_t cache_id) {
    bool copies_exist = false;
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        // The operation on copies_exist is bit-OR, so we can make use of short-circuit AND
        copies_exist |= i != cache_id && caches[i] && caches[i]->receiveBusMsg(bus_msg, addr);
    return copies_exist;
}

void MemoryBus::printStats() {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        if (caches[i])
            caches[i]->printStats();
}
