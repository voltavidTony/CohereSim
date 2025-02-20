/// @file memory_system.cc
/// @brief Implementation of the MemorySystem class methods

#include "cache.h"
#include "memory_system.h"

MemorySystem::MemorySystem(cache_config& config)
    : copies_exist(false), flushed(false), caches{ 0 }, config(config) {}
MemorySystem::~MemorySystem() {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        delete caches[i];
}

void MemorySystem::issuePrRd(addr_t addr, uint32_t cache_id
#ifdef WRITE_TIMESTAMP
    , size_t read_timestamp
#endif
) {
    // Dynamically allocate cache
    if (!caches[cache_id]) caches[cache_id] = new Cache(*this, cache_id, config);

#ifdef WRITE_TIMESTAMP
    access_timestamp = read_timestamp;
#endif
    caches[cache_id]->receivePrRd(addr);
#ifdef WRITE_TIMESTAMP
    verifyTimestamp(addr, false, read_timestamp);
#endif
}

void MemorySystem::issuePrWr(addr_t addr, uint32_t cache_id
#ifdef WRITE_TIMESTAMP
    , size_t write_timestamp
#endif
) {
    // Dynamically allocate cache
    if (!caches[cache_id]) caches[cache_id] = new Cache(*this, cache_id, config);

#ifdef WRITE_TIMESTAMP
    access_timestamp = write_timestamp;
#endif
    caches[cache_id]->receivePrWr(addr);
#ifdef WRITE_TIMESTAMP
    verifyTimestamp(addr, true, write_timestamp);
#endif
}

void MemorySystem::printStats() {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        if (caches[i])
            caches[i]->printStats();
}

#ifdef WRITE_TIMESTAMP
void MemorySystem::verifyTimestamp(addr_t addr, bool write, size_t current_timestamp) {
    // Collect each cache's timestamps for the accessed address
    size_t timestamps[MAX_N_CACHES];
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        timestamps[i] = caches[i] ? caches[i]->getTimestamp(addr) : 0;

    // Determine if any of the timestamps differ and keep track of the maximum
    bool discrepancy = false;
    size_t max_timestamp = 0;
    for (uint32_t i = 0; i < MAX_N_CACHES; i++) {
        if (!timestamps[i]) continue;
        if (timestamps[i] != max_timestamp) discrepancy = (bool)max_timestamp;
        if (timestamps[i] > max_timestamp) max_timestamp = timestamps[i];
    }

    if (discrepancy) {
        // Report the discrepancy
        std::cerr << "Cache lines out of date after " << (write ? "writing to" : "reading from");
        std::cerr << " address " << std::setbase(16) << addr;
        std::cerr << " at step " << std::setbase(10) << current_timestamp + 1 << ": ";

        // Print the first discrepancy
        uint32_t i = 0;
        for (; i < MAX_N_CACHES; i++)
            if (timestamps[i] && timestamps[i] < max_timestamp) {
                std::cerr << i;
                break;
            }

        // Print the remaining discrepancies
        for (; i < MAX_N_CACHES; i++)
            if (timestamps[i] && timestamps[i] < max_timestamp)
                std::cerr << ", " << i;
        std::cerr << std::endl;
    }
}
#endif
