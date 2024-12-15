/// @file memory_bus.h
/// @brief Declaration of the MemoryBus class

#pragma once

#include "typedefs.h"

/// @brief The MemoryBus class connecting multiple caches and main memory
class MemoryBus {
public:

    /// @brief Construct a new memory bus
    /// @param config The configuration of this memory bus
    MemoryBus(cache_config& config);
    ~MemoryBus();

    /// @brief Issue a PrWr message to a cache
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the recipient
    void issuePrRd(addr_t addr, uint32_t cache_id);
    /// @brief Issue a PrRd message to a cache
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the recipient
    void issuePrWr(addr_t addr, uint32_t cache_id);

    /// @brief Issue a bus message from a cache to all other caches
    /// @param bus_msg The specific bus message
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the requestor
    /// @return True if the 'COPIES-EXIST' line was asserted
    bool issueBusMsg(bus_msg_e bus_msg, addr_t addr, uint32_t cache_id);

    /// @brief Print simulation run statistics in CSV format (headerless)
    void printStats();

private:

    /// @brief Array of this memory bus's caches
    Cache** caches;

    /// @brief Config for this memory bus
    cache_config config;

    /// @brief Flag to indicate if a cache flushed one of its lines
    bool flushed;

    uint32_t tag_offset;
};
