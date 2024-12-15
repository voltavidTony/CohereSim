/// @file fifo.h
/// @brief Declaration of the FIFO replacement policy

#pragma once

#include "replacement_policy.h"

/// @brief The FIFO replacement policy
class FIFO : public ReplacementPolicy {
public:

    /// @brief Construct a new FIFO replacement policy
    /// @param cache The parent cache
    /// @param num_sets The number of sets in the cache
    /// @param assoc The associativity of the chace
    FIFO(CacheABC& cache, uint32_t num_sets, uint32_t assoc);
    ~FIFO();

    /// @brief Determine which line of a range of lines to replace
    /// @param set_idx The index of the set to choose from
    /// @return The chosen line's index within the set (0 to assoc-1)
    uint32_t getVictim(uint32_t set_idx);

    /// @brief Print out the replacer's internal state
    /// @param set_idx The index of the set
    void printState(uint32_t set_idx);

private:

    /// @brief The index of the next line to evict
    uint32_t* up_next;
};
