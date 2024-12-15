/// @file textbook_mode.h
/// @brief Declaration of the TextbookMode base class

#pragma once

#include "cache_abc.h"
#include "typedefs.h"

/// @brief The TextbookMode abstract base class
class TextbookMode : public CacheABC {
public:

    /// @brief Construct a new textbook mode cache
    /// @param p_name The policy/protocol name
    TextbookMode(std::string p_name) : p_name(p_name) {}
    /// @brief Finalize the textbook mode output
    virtual ~TextbookMode() {}

    /// @brief Evaluate a command received from the input
    /// @param cmd The input command
    /// @return True if the command was successful
    virtual bool evalutateCommand(std::string& cmd) = 0;

    /// @brief Issue a BusRd message to neighboring caches
    /// @param bus_msg The specific bus message
    /// @param addr The address accessed
    /// @return True if the 'COPIES-EXIST' line was asserted
    virtual bool issueBusMsg(bus_msg_e bus_msg, addr_t addr) { return false; }

    /// @brief Get the state of a line in the cache
    /// @param set_idx The index of the set containing the line
    /// @param way_idx The index of the way containing the line (0 to assoc-1)
    /// @return The state of the cache line
    state_e getLineState(uint32_t set_idx, uint32_t way_idx) { return lines[way_idx].state; }

    /// @brief Write the command format message to stderr
    virtual void printCmdFormatMessage() = 0;

protected:

    /// @brief The name of the policy/protocol being run in textbook mode
    std::string p_name;

    /// @brief The cache lines in textbook mode
    cache_line lines[N_TEXTBOOK_LINES];

private:

    /// @brief Revert the system back to the initial state
    virtual void reset() = 0;

    /// @brief Print a horizontal line in the table
    virtual void printSeparator() = 0;
    /// @brief Print results after each operation to the table
    virtual void printStats() = 0;
};
