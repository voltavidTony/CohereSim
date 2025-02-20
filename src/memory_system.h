/// @file memory_system.h
/// @brief Declaration of the MemorySystem class

#pragma once

#include "typedefs.h"

/// @brief The maximum number of caches supported by the current trace format (7 bit ID = 128 ID values)
#define MAX_N_CACHES 0b10000000

/// @brief The MemorySystem class connecting multiple caches and main memory
class MemorySystem {
public:

#ifdef WRITE_TIMESTAMP
    /// @brief The access number of the current memory access
    size_t access_timestamp;

    /// @brief The most recent timestamp of a cache block across all caches
    size_t most_recent_sibling;
#endif

    /// @brief Flag to indicate if copies of a cache block exist in other caches
    bool copies_exist;

    /// @brief Flag to indicate if a cache flushed one of its lines
    bool flushed;

    /// @brief Construct a new memory system
    /// @param config The configuration of this memory system
    MemorySystem(cache_config& config);
    virtual ~MemorySystem();

    /// @brief Issue a PrWr message to a cache
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the recipient
#ifdef WRITE_TIMESTAMP
    /// @param read_timestamp The access number of the current read access
#endif
    void issuePrRd(addr_t addr, uint32_t cache_id
#ifdef WRITE_TIMESTAMP
        , size_t read_timestamp
#endif
    );
    /// @brief Issue a PrRd message to a cache
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the recipient
#ifdef WRITE_TIMESTAMP
    /// @param write_timestamp The access number of the current write access
#endif
    void issuePrWr(addr_t addr, uint32_t cache_id
#ifdef WRITE_TIMESTAMP
        , size_t write_timestamp
#endif
    );

    /// @brief Issue a bus message from a cache to all other caches
    /// @param bus_msg The specific bus message
    /// @param addr The address accessed
    /// @param cache_id The cache ID of the requestor
    virtual void issueBusMsg(bus_msg_e bus_msg, addr_t addr, uint32_t cache_id) = 0;

    /// @brief Print simulation run statistics in CSV format (headerless)
    void printStats();

protected:

    /// @brief Array of this memory system's caches
    Cache* caches[MAX_N_CACHES];

private:

    /// @brief Config for this memory system
    cache_config config;

#ifdef WRITE_TIMESTAMP
    /// @brief Check if all valid copies of a cache block have the same timestamp
    /// @param addr The memory address in the cache block
    /// @param write Whether the current operation is a processor write
    /// @param current_timestamp The current access number
    void verifyTimestamp(addr_t addr, bool write, size_t current_timestamp);
#endif
};

/// @brief Create a mapping in 'directory_map' from a string containing the class name to a factory method for the class
/// @param dir_prot The directory protocol type
#define ADD_DIRECTORY_TO_CMD_LINE(dir_prot) static int register_directory = []() { \
if (directory_map == nullptr) directory_map = new std::map<std::string, dir_factory_t, ci_less>(); \
(*directory_map)[#dir_prot] = [](cache_config& config) { return new dir_prot(config); }; return 0; }()
