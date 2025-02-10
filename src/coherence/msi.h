/// @file msi.h
/// @brief Declaration of the MSI coherence protocol

#pragma once

#include "coherence_protocol.h"

/// @brief The MSI coherence protocol
class MSI : public CoherenceProtocol {
public:

    /// @brief Construct a new MSI coherence protocol
    /// @param cache The parent cache
    MSI(CacheABC& cache) : CoherenceProtocol(cache) {}

    /// @brief Receive a PrRd message
    /// @param line The cache line accessed (non-null)
    void PrRd(cache_line* line);
    /// @brief Receive a PrWr message
    /// @param line The cache line accessed (non-null)
    void PrWr(cache_line* line);

    /// @brief Receive a BusRd message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    bool BusRd(cache_line* line);
    /// @brief Receive a BusRdX message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    bool BusRdX(cache_line* line);

    /// @brief Determine whether a line needs to be written back to main memory
    /// @param state The state of the line
    /// @return Whether the line needs to be written back to main memory
    bool isWriteBackNeeded(state_e state);
};
