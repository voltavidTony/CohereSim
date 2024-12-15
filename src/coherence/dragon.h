/// @file dragon.h
/// @brief Declaration of the Dragon coherence protocol

#pragma once

#include "coherence_protocol.h"

/// @brief The Dragon coherence protocol
class Dragon : public CoherenceProtocol {
public:

    /// @brief Construct a new Dragon coherence protocol
    /// @param cache The parent cache
    Dragon(CacheABC& cache) : CoherenceProtocol(cache) {}

    /// @brief Receive a PrRd message
    /// @param addr The address accessed
    /// @param line The cache line accessed (non-null)
    void PrRd(addr_t addr, cache_line* line);
    /// @brief Receive a PrWr message
    /// @param addr The address accessed
    /// @param line The cache line accessed (non-null)
    void PrWr(addr_t addr, cache_line* line);

    /// @brief Receive a BusRd message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    bool BusRd(cache_line* line);
    /// @brief Receive a BusUpdt message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    bool BusUpdt(cache_line* line);

    /// @brief Determine whether the coherence protocol does dirty sharing
    /// @return True if the coherence protocol does dirty sharing
    bool doesDirtySharing();

    /// @brief Determine whether a line needs to be written back to main memory
    /// @param state The state of the line
    /// @return Whether the line needs to be written back to main memory
    bool isWriteBackNeeded(state_e state);
};
