/// @file rr.h
/// @brief Declaration of the random replacement policy

#pragma once

#include "replacement_policy.h"

/// @brief The RR replacement policy
class RR : public ReplacementPolicy {
public:

    /// @brief Construct a new replacement policy
    /// @param cache The parent cache
    /// @param num_sets The number of sets in the cache
    /// @param assoc The associativity of the chace
    RR(CacheABC& cache, uint32_t num_sets, uint32_t assoc);

    /// @brief Determine which line of a range of lines to replace
    /// @param set_idx The index of the set to choose from
    /// @return The chosen line's index within the set (0 to assoc-1)
    uint32_t getVictim(uint32_t set_idx);
};
