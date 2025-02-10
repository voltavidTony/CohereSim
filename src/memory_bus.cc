/// @file memory_bus.cc
/// @brief Implementation of the MemoryBus class methods

#include "cache.h"
#include "memory_bus.h"

MemoryBus::MemoryBus(cache_config& config)
    : copies_exist(false), flushed(false), caches{ 0 }, config(config) {}
MemoryBus::~MemoryBus() {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        delete caches[i];
}

void MemoryBus::issuePrRd(addr_t addr, uint32_t cache_id) {
    // Dynamically allocate cache
    if (!caches[cache_id]) caches[cache_id] = new Cache(*this, cache_id, config);

    caches[cache_id]->receivePrRd(addr);
}

void MemoryBus::issuePrWr(addr_t addr, uint32_t cache_id) {
    // Dynamically allocate cache
    if (!caches[cache_id]) caches[cache_id] = new Cache(*this, cache_id, config);

    caches[cache_id]->receivePrWr(addr);
}

void MemoryBus::issueBusMsg(bus_msg_e bus_msg, addr_t addr, uint32_t cache_id) {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        // The operation on copies_exist is bit-OR, so we can make use of short-circuit AND
        if (i != cache_id && caches[i])
            caches[i]->receiveBusMsg(bus_msg, addr);
}

void MemoryBus::printStats() {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        if (caches[i])
            caches[i]->printStats();
}
