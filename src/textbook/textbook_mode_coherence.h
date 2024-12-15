/// @file textbook_mode_coherence.h
/// @brief Declaration of the TextbookModeCoherence and BusEvent classes

#pragma once

#include <vector>

#include "textbook_mode.h"

/// @brief The BusEvent class
struct bus_event {
    /// @brief The bus event that occurred
    uint32_t event;
    /// @brief The id of the resonsible cache
    uint32_t issuer;

    /// @brief Create a new bus event
    /// @param bmsg The bus event that occurred
    /// @param issuer The id of the responsible cache
    bus_event(bus_msg_e bmsg, uint32_t issuer);
    /// @brief Create a new bus event
    /// @param stat The bus event that occurred
    /// @param issuer The id of the responsible cache
    bus_event(statistic_e stat, uint32_t issuer);
};

/// @brief The TextbookMode class for coherence protocols
class TextbookModeCoherence : public TextbookMode {
public:

    /// @brief Construct a new textbook mode cache showcasing a coherence protocol
    /// @param coherence_protocol_name The coherence protocol to use
    TextbookModeCoherence(std::string coherence_protocol_name);
    /// @brief Finalize the textbook mode output
    ~TextbookModeCoherence();

    /// @brief Evaluate a command received from the input
    /// @param cmd The input command
    /// @return True if the command was successful
    bool evalutateCommand(std::string& cmd);

    /// @brief Issue a BusRd message to "neighboring caches"
    /// @param bus_msg The specific bus message
    /// @param addr The address accessed
    /// @return True if the 'COPIES-EXIST' line was asserted
    bool issueBusMsg(bus_msg_e bus_msg, addr_t addr);

    /// @brief Write the command format message to stderr
    void printCmdFormatMessage();

private:

    /// @brief Coherence protocol used by this cache
    CoherenceProtocol* coherence_protocol;

    /// @brief Vector for holding bus events (including processor read and write).
    /// Statistics are individual, rather than cumulative
    std::vector<bus_event> bus_events;

    /// @brief The most recent command issued
    bus_event command;

    /// @brief Issue an Evict message to a cache
    /// @param cache_id The cache ID of the recipient
    void receiveEvict(uint32_t cache_id);
    /// @brief Issue a PrWr message to a cache
    /// @param cache_id The cache ID of the recipient
    void receivePrRd(uint32_t cache_id);
    /// @brief Issue a PrRd message to a cache
    /// @param cache_id The cache ID of the recipient
    void receivePrWr(uint32_t cache_id);

    /// @brief Issue a bus message to "this cache"
    /// @param bus_msg The specific bus message
    /// @param cache_id The cache ID of the recipient
    /// @return True if the accessed line was flushed into the bus
    bool receiveBusMsg(bus_msg_e bus_msg, uint32_t cache_id);

    /// @brief Revert the system back to the initial state
    void reset();

    /// @brief Print a horizontal line in the table
    void printSeparator();
    /// @brief Print bus events and results of operation
    void printStats();
};
