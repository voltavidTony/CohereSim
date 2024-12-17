/// @file replacement_policy.h
/// @brief Declaration of the replacement policy base class

#pragma once

#include "cache_abc.h"

/// @brief The base class for replacement policies
class ReplacementPolicy {
public:

    /// @brief Construct a new replacement policy
    /// @param cache The parent cache
    /// @param num_sets The number of sets in the cache
    /// @param assoc The associativity of the chace
    ReplacementPolicy(CacheABC& cache, uint32_t num_sets, uint32_t assoc)
        : cache(cache), num_sets(num_sets), assoc(assoc) {}
    virtual ~ReplacementPolicy() {}

    /// @brief Determine which line of a range of lines to replace
    /// @param set_idx The index of the set to choose from
    /// @return The chosen line's way (0 to assoc-1)
    virtual uint32_t getVictim(uint32_t set_idx) { return 0; }

    /// @brief Notify the replacement policy that a line was just accessed
    /// @param set_idx The index of the set containing the line
    /// @param way_idx The index of the way containing the line (0 to assoc-1)
    virtual void touch(uint32_t set_idx, uint32_t way_idx) {}

    /// @brief Print out the replacer's internal state
    /// @param set_idx The index of the set
    virtual void printState(uint32_t set_idx) {}

protected:

    /// @brief The parent cache
    CacheABC& cache;

    /// @brief The number of sets in the cache
    uint32_t num_sets;
    /// @brief The associativity of the cache
    uint32_t assoc;
};

/// @brief Create a mapping in 'replacement_map' from a string containing the class name to a factory method for the class
/// @param rep_pol The replacement policy type
#define ADD_REPLACER_TO_CMD_LINE(rep_pol) static int register_replacement = []() { \
if (replacement_map == nullptr) replacement_map = new std::map<std::string, rep_factory_t, ci_less>(); \
(*replacement_map)[#rep_pol] = [](CacheABC& cache, uint32_t num_sets, uint32_t assoc) \
{ return new rep_pol(cache, num_sets, assoc); }; return 0; }()
