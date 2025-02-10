/// @file cache_abc.h
/// @brief Declaration of the cache interface class

#pragma once

#include "typedefs.h"

/// @brief The cache interface
class CacheABC {
public:

    virtual ~CacheABC() {}

    /// @brief Issue a BusRd message to neighboring caches
    /// @param bus_msg The specific bus message
    /// @return True if the 'COPIES-EXIST' line was asserted
    virtual bool issueBusMsg(bus_msg_e bus_msg) = 0;

    /// @brief Get the state of a line in the cache
    /// @param set_idx The index of the set containing the line
    /// @param way_idx The index of the way containing the line (0 to assoc-1)
    /// @return The state of the cache line
    virtual state_e getLineState(uint32_t set_idx, uint32_t way_idx) = 0;
};
