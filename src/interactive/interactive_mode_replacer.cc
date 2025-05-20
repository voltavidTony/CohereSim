/// @file interactive_mode_replacer.cc
/// @brief Implementation of the InteractiveModeReplacer class

#include "replacement_policy.h"
#include "interactive_mode_replacer.h"

/// @brief Table column widths
enum col_width_e {
    /// @brief String length of the word "Accessed"
    COL_WIDTH_ACCESS = 8,
    /// @brief String length of the word "Victim"
    COL_WIDTH_VICTIM = 6,
    /// @brief Space for the tags of all caches
    COL_WIDTH_TAGS = 2 * N_INTERACTIVE_MODE_LINES - 1,
    /// @brief String length of the words "Replacer State" + 1
    COL_WIDTH_REP_STATE = 15
};

InteractiveModeReplacer::InteractiveModeReplacer(std::string replacement_policy_name)
    : InteractiveMode(replacement_policy_name), accessee(' '), victim(' ') {
    // Initialize cache components
    replacement_policy = (*replacement_map)[p_name](*this, 1, N_INTERACTIVE_MODE_LINES);

    // Initialize cache lines
    for (uint32_t i = 0; i < N_INTERACTIVE_MODE_LINES; i++) {
        lines[i].tag = 0;
        lines[i].state = I;
    }

    // Print table header
    std::cout << std::left << std::setw(COL_WIDTH_ACCESS) << "Accessed" << " | ";
    std::cout << std::left << std::setw(COL_WIDTH_VICTIM) << "Victim" << " | ";
    std::cout << std::left << std::setw(COL_WIDTH_TAGS) << "Tags" << " | ";
    std::cout << std::left << std::setw(COL_WIDTH_REP_STATE) << "Replacer State" << std::endl;
    printSeparator();
    printStats();
}
InteractiveModeReplacer::~InteractiveModeReplacer() {
    delete replacement_policy;
    // Close out the table
    printSeparator();
}

bool InteractiveModeReplacer::evalutateCommand(std::string& cmd) {
    // Only accept one letter commands
    if (cmd.length() != 1) return false;

    // Reset command
    if (cmd[0] == '-') {
        reset();
        return true;
    }

    // Access command
    tag_t tag = std::toupper(cmd[0]);
    if ('A' <= tag && tag <= 'Z') {
        receiveAccess(tag);
        printStats();
        return true;
    }

    return false;
}

void InteractiveModeReplacer::printCmdFormatMessage() {
    std::cerr << "Command must be a letter between 'A' and 'Z' or '-'" << std::endl;
}

void InteractiveModeReplacer::receiveAccess(tag_t tag) {
    accessee = tag;
    victim = ' ';

    /*
     * Code copied and adapted from Cache::allocate, Cache::findLine, and Cache::receivePrRd
     * Main changes are the removal of statistics and coherence protocol involvement
     */

     // Find line
    uint32_t line_idx = N_INTERACTIVE_MODE_LINES;
    for (uint32_t i = 0; i < N_INTERACTIVE_MODE_LINES; i++)
        if (lines[i].tag == tag) {
            line_idx = i;
            break;
        }

    // Allocate line if not found
    if (line_idx == N_INTERACTIVE_MODE_LINES) {
        line_idx = replacement_policy->getVictim(0);
        victim = lines[line_idx].state ? std::toupper(lines[line_idx].tag) : ' ';
        lines[line_idx].tag = tag;
        lines[line_idx].state = V;
    }

    // Record line access
    replacement_policy->touch(0, line_idx);

}

void InteractiveModeReplacer::reset() {
    // Reset attributes
    accessee = ' ';
    victim = ' ';
    delete replacement_policy;
    replacement_policy = (*replacement_map)[p_name](*this, 1, N_INTERACTIVE_MODE_LINES);

    // Reset cache lines
    for (uint32_t i = 0; i < N_INTERACTIVE_MODE_LINES; i++) {
        lines[i].tag = 0;
        lines[i].state = I;
    }

    // Restart table
    printSeparator();
    printStats();
}

void InteractiveModeReplacer::printSeparator() {
    // Fill each column with dashes, the separator is a plus ('+')
    for (uint32_t i = 0; i < COL_WIDTH_ACCESS; i++)
        std::cout << '-';
    std::cout << "-+-";
    for (uint32_t i = 0; i < COL_WIDTH_VICTIM; i++)
        std::cout << '-';
    std::cout << "-+-";
    for (uint32_t i = 0; i < COL_WIDTH_TAGS; i++)
        std::cout << '-';
    std::cout << "-+-";
    for (uint32_t i = 0; i < COL_WIDTH_REP_STATE; i++)
        std::cout << '-';
    std::cout << std::endl;
}
void InteractiveModeReplacer::printStats() {
    // Print the accessed line in the first column
    std::cout << std::setw(COL_WIDTH_ACCESS) << accessee << " | ";

    // Print the victim line in the second column
    std::cout << std::setw(COL_WIDTH_VICTIM) << victim << " | ";

    // Print resulting line tags
    std::cout << (lines[0].state ? (char)lines[0].tag : '-');
    for (uint32_t i = 1; i < N_INTERACTIVE_MODE_LINES; i++)
        std::cout << " " << (lines[i].state ? (char)lines[i].tag : '-');
    std::cout << " | ";

    // Print resulting internal replacer state
    replacement_policy->printState(0);
    std::cout << std::endl;
}
