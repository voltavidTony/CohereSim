/// @file broadcast.h
/// @brief Declaration of the Broadcast directory protocol

#pragma once

#include "memory_system.h"

/// @brief The Broadcast directory protocol
class Broadcast : public MemorySystem {
public:

    /// @brief Construct a new Broadcast directory protocol
    /// @param config The configuration of the memory system
    Broadcast(cache_config& config) : MemorySystem(config) {}

    /// @brief Issue a bus message from a cache to all other caches
    /// @param bus_msg The specific bus message
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the requestor
    void issueBusMsg(bus_msg_e bus_msg, addr_t addr, uint32_t cache_id);
};
