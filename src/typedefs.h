/// @file typedefs.h
/// @brief Global type definitions

#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>

/// @brief The maximum number of caches supported by the current trace format (7 bit ID = 128 ID values)
#define MAX_N_CACHES 0b10000000

/// @brief The number of cache lines that textbook mode uses
#define N_TEXTBOOK_LINES 5
#if N_TEXTBOOK_LINES > 9
#error("N_TEXTBOOK_LINES must be a single digit number")
#endif

/// @brief Determine the position of the MSB (for int32, a range of 0-31)
#define MSB(x) (8 * sizeof(x) - 1 - __builtin_clz(x))

/// @brief Cache class
class Cache;
/// @brief Cache abstract base class
class CacheABC;
/// @brief Coherence protocol base class
class CoherenceProtocol;
/// @brief Memory bus class
class MemoryBus;
/// @brief Replacement policy base class
class ReplacementPolicy;

/// @brief Cache line state
enum state_e {
    /// @brief Alias for invalid, used in protocols that don't invalidate
    Unallocated = 0,
    /// @brief Invalid
    I = 0,

    /* Exclusive states */
    /// @brief Dirty exclusive
    D,
    /// @brief Clean exclusive
    E,
    /// @brief Modified exclusive
    M,
    /// @brief Valid exclusive
    V,

    /* Shared states */
    /// @brief Owned
    O,
    /// @brief Shared
    S,
    /// @brief Shared clean
    Sc,
    /// @brief Shared modified
    Sm
};

/// @brief Bus message IDs
enum bus_msg_e {
    /// @brief Read access on a cache line
    ProcRead,
    /// @brief Write access on a cache line
    ProcWrite,

    /// @brief Bus read message issued by a cache
    BusRead,
    /// @brief Bus read-exclusive message issued by a cache
    BusReadX,
    /// @brief Bus update message issued by a cache
    BusUpdate,
    /// @brief Bus write message issued by a cache
    BusUpgrade,
    /// @brief Bus upgrade message issued by a cache
    BusWrite,

    /// @brief Number of bus messages
    N_MESSAGES
};

/// @brief Cache runtime statistic IDs
/// @note Continuation of 'bus_msg_e'
enum statistic_e {
    /// @brief Read miss on a cache line
    ReadMiss = N_MESSAGES,  // Continuation of 'bus_msg_e'
    /// @brief Write miss on a cache line
    WriteMiss,

    /// @brief Cache line data broadcasted across memory bus
    LineFlush,
    /// @brief Cache line data retrieved from main memory
    LineFetch,
    /// @brief Transfer between two caches
    CacheToCache,
    /// @brief Cache line data written to main memory
    WriteBack,
    /// @brief Direct write from CPU to main memory
    WriteMemory,

    /// @brief Cache line evicted by the replacement policy
    Eviction,

    /// @brief Cache line changes from shared (O, S, Sc, Sm) to singular (D, E, M, V)
    Exclusion,
    /// @brief Cache line changes from singular (D, E, M, V) to shared (O, S, Sc, Sm)
    Intervention,
    /// @brief Cache line state set to invalid (I)
    Invalidation,

    /// @brief The number of statistics a cache keeps track of; not a statistic
    N_STATISTICS
};

/// @brief Memory address
typedef uint32_t addr_t;
/// @brief Cache line tag
typedef uint32_t tag_t;

/// @brief Coherence protocol factory function signature
typedef std::function<CoherenceProtocol* (CacheABC&)> coh_factory_t;
/// @brief Replacement policy factory function signature
typedef std::function<ReplacementPolicy* (CacheABC&, uint32_t, uint32_t)> rep_factory_t;

/// @brief Cache line fields (without data field)
struct cache_line {
    /// @brief Tag of the line
    tag_t tag;
    /// @brief State the line is in
    state_e state;
};

/// @brief Configuration for the caches in an instance of MemoryBus
struct cache_config {
    /// @brief The id for this configuration
    uint32_t id;
    /// @brief The size of each L1 cache
    uint32_t cache_size;
    /// @brief The line size of each L1 cache
    uint32_t line_size;
    /// @brief The associativity of each L1 cache
    uint32_t assoc;
    /// @brief The name of the coherence protocol
    std::string coherence;
    /// @brief The name of the replacement policy
    std::string replacer;
};

/// @brief Comparator functor for strings, case insensitive
struct ci_less {
    /// @brief Compare two string ignoring case
    /// @param s1 The first string
    /// @param s2 The second string
    /// @return True if 's1' comes lexicographically before 's2'
    bool operator() (const std::string& s1, const std::string& s2) const;
};

/// @brief A map from coherence protocol names to their factory functions
extern std::map<std::string, coh_factory_t, ci_less>* coherence_map;
/// @brief A map from replacement policy names to their factory functions
extern std::map<std::string, rep_factory_t, ci_less>* replacement_map;
