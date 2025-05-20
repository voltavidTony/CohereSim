/// @file interactive_mode_replacer.h
/// @brief Declaration of the InteractiveModeReplacer class

#pragma once

#include "interactive_mode.h"

/// @brief The InteractiveMode class for replacement policies
class InteractiveModeReplacer : public InteractiveMode {
public:

    /// @brief Construct a new interactive mode cache showcasing a replacement policy
    /// @param replacement_policy_name The replacement policy to use
    InteractiveModeReplacer(std::string replacement_policy_name);
    /// @brief Finalize the interactive mode output
    ~InteractiveModeReplacer();

    /// @brief Evaluate a command received from the input
    /// @param cmd The input command
    /// @return True if the command was successful
    bool evalutateCommand(std::string& cmd);

    /// @brief Write the command format message to stderr
    void printCmdFormatMessage();

private:

    /// @brief Issue an access to a cache block
    /// @param tag The tag of the block
    void receiveAccess(tag_t tag);

    /// @brief Revert the system back to the initial state
    void reset();

    /// @brief Print a horizontal line in the table
    void printSeparator();
    /// @brief Print simulation run statistics in CSV format (headerless)
    void printStats();

    /// @brief Coherence protocol used by this cache
    ReplacementPolicy* replacement_policy;

    /// @brief The most recent command issued
    char accessee;
    /// @brief The most recent victim
    char victim;
};
