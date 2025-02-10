/// @file coherence_protocol.h
/// @brief Definition of the coherence protocol base class

#pragma once

#include "cache_abc.h"

/// @brief Error message printed when a cache line is in a state not supported by the coherence protocol
#define STATE_ERR std::cerr << __FILE__ << ':' << __FUNCTION__ << ": Cache line in invalid state: " << (int)(line->state) << std::endl
/// @brief Error message printed when a cache issues a bus message not supported by the coherence protocol
#define UNIMPLEMENTED std::cerr << "Call of unimplemented bus message: " << __FUNCTION__ << std::endl; return false

/// @brief The base class for coherence protocols
class CoherenceProtocol {
public:

    /// @brief Construct a new coherence protocol
    /// @param cache The parent cache
    CoherenceProtocol(CacheABC& cache) : cache(cache) {}
    virtual ~CoherenceProtocol() {}

    /// @brief Receive a PrRd message
    /// @param line The cache line accessed (non-null)
    virtual void PrRd(cache_line* line) = 0;
    /// @brief Receive a PrWr message
    /// @param line The cache line accessed
    virtual void PrWr(cache_line* line) = 0;

    /// @brief Receive a BusRd message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    virtual bool BusRd(cache_line* line) = 0;
    /// @brief Receive a BusRdX message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    virtual bool BusRdX(cache_line* line) { UNIMPLEMENTED; };
    /// @brief Receive a BusUpdt message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    virtual bool BusUpdt(cache_line* line) { UNIMPLEMENTED; };
    /// @brief Receive a BusUpgr message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    virtual bool BusUpgr(cache_line* line) { UNIMPLEMENTED; };
    /// @brief Receive a BusWr message
    /// @param line The cache line accessed
    /// @return True if the line was flushed to the bus
    virtual bool BusWr(cache_line* line) { UNIMPLEMENTED; };

    /// @brief Determine whether the coherence protocol does dirty sharing
    /// @return True if the coherence protocol does dirty sharing
    virtual bool doesDirtySharing() { return false; }
    /// @brief Determine whether the coherence protocol uses write no-allocate
    /// @return True if the coherence protocol uses write no-allocate
    virtual bool doesWriteNoAllocate() { return false; }

    /// @brief Determine whether a line needs to be written back to main memory
    /// @param state The state of the line
    /// @return Whether the line needs to be written back to main memory
    virtual bool isWriteBackNeeded(state_e state) = 0;

protected:

    /// @brief The parent cache
    CacheABC& cache;
};

/// @brief Create a mapping in 'coherence_map' from a string containing the class name to a factory method for the class
/// @param coh_prot The coherence protocol type
#define ADD_COHERENCE_TO_CMD_LINE(coh_prot) static int register_coherence = []() { \
if (coherence_map == nullptr) coherence_map = new std::map<std::string, coh_factory_t, ci_less>(); \
(*coherence_map)[#coh_prot] = [](CacheABC& cache) { return new coh_prot(cache); }; return 0; }()
