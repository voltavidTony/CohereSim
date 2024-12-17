/// @file textbook_mode_coherence.cc
/// @brief Implementation of the TextbookModeCoherence and BusEvent classes

#include "coherence_protocol.h"
#include "textbook_mode_coherence.h"

/// @brief Dummy tag value indicating that a line is allocated
#define ALLOCATED 0x55555555

/// @brief String names of the states in state_e
constexpr const char* state_names[] = {
    " I ", " D ", " E ", " M ", " V ", " O ", " S ", " Sc", " Sm"
};
/// @brief String names of bus messages and statistics in bus_msg_e and statistic_e
constexpr const char* bus_event_names[] = {
    "PrRd", "PrWr", "BusRd", "BusRdX", "BusUpdt", "BusUpgr", "BusWr", "Read Miss",
    "Write Miss", "Line Flush", "Line Fetch", "Cache to Cache", "Write Back", "Write Memory"
};

/// @brief Table column widths
enum col_width_e {
    /// @brief Operations have one letter and one digit
    COL_WIDTH_OP = 2,
    /// @brief String length of the longest bus event name + 2
    COL_WIDTH_EVENT = 16,
    /// @brief String length of "Main Memory" or "Data Source"
    COL_WIDTH_SOURCE = 11,
    /// @brief Space for the states of all caches
    COL_WIDTH_STATES = 3 * N_TEXTBOOK_LINES - 1
};

bus_event::bus_event(bus_msg_e bmsg, uint32_t issuer) : event(bmsg), issuer(issuer) {}
bus_event::bus_event(statistic_e stat, uint32_t issuer) : event(stat), issuer(issuer) {}

TextbookModeCoherence::TextbookModeCoherence(std::string coherence_protocol_name)
    : TextbookMode(coherence_protocol_name), command(Invalidation, 0) { // Here Invalidation is used to represent the lack of command
    // Initialize cache components
    coherence_protocol = (*coherence_map)[p_name](*this);

    // Initialize cache lines
    for (uint32_t i = 0; i < N_TEXTBOOK_LINES; i++) {
        lines[i].tag = 0;
        lines[i].state = I;
    }

    // Print table header
    std::cout << std::left << std::setw(COL_WIDTH_OP) << "OP" << " | ";
    std::cout << std::left << std::setw(COL_WIDTH_EVENT) << "Bus Event" << " | ";
    std::cout << std::left << std::setw(COL_WIDTH_SOURCE) << "Data Source" << " | ";
    std::cout << std::left << std::setw(COL_WIDTH_STATES) << "States" << std::endl;
    printSeparator();
    printStats();
}
TextbookModeCoherence::~TextbookModeCoherence() {
    delete coherence_protocol;
    // Close out the table
    printSeparator();
}

bool TextbookModeCoherence::evalutateCommand(std::string& cmd) {
    // Reset command (R is already taken; X for clear)
    if (cmd.length() == 1 && (cmd[0] == 'x' || cmd[0] == 'X')) {
        reset();
        return true;
    }

    if (cmd.length() == 2 && '1' <= cmd[1] && cmd[1] <= ('0' + N_TEXTBOOK_LINES)) {
        switch (cmd[0]) {
        case 'e':
        case 'E': // Evict command
            receiveEvict(cmd[1] - '1');
            break;
        case 'r':
        case 'R': // Read command
            receivePrRd(cmd[1] - '1');
            break;
        case 'w':
        case 'W': // Write command
            receivePrWr(cmd[1] - '1');
            break;
        default:
            return false;
        }
        printStats();
        return true;
    }

    return false;
}

bool TextbookModeCoherence::issueBusMsg(bus_msg_e bus_msg, addr_t addr) {
    bus_events.emplace_back(bus_msg, command.issuer);

    bool copies = false;
    bool flushed = false;
    switch (bus_msg) {
    case BusRead:
    case BusReadX:
    case BusUpdate:
    case BusUpgrade:
    case BusWrite:
        // Send the bus message to each cache, keeping track of if copies exist and if any line was flushed
        for (uint32_t i = 0; i < N_TEXTBOOK_LINES; i++)
            if (i != command.issuer && lines[i].state) {
                if (receiveBusMsg(bus_msg, i)) {
                    bus_events.emplace_back(LineFlush, i);
                    flushed = true;
                }
                copies = true;
            }
        break;
    default: // Only respond to actual bus messages (enum has other values)
        break;
    }
    // Figure out where the cache line was read in from
    if ((bus_msg == BusRead || bus_msg == BusReadX)) {
        if (flushed) bus_events.emplace_back(CacheToCache, command.issuer);
        else bus_events.emplace_back(LineFetch, N_TEXTBOOK_LINES);
    }
    return copies;
}
bool TextbookModeCoherence::receiveBusMsg(bus_msg_e bus_msg, uint32_t cache_id) {
    // Map bus_msg_e to the appropriate function call
    switch (bus_msg) {
    case BusRead: {
        // The BusRead message requires extra logic for determining when a WriteBack occurs
        state_e prev_state = lines[cache_id].state;
        bool flushed = coherence_protocol->BusRd(&lines[cache_id]);
        if (!coherence_protocol->doesDirtySharing() && coherence_protocol->isWriteBackNeeded(prev_state))
            bus_events.emplace_back(WriteBack, cache_id);
        return flushed;
    }
    case BusReadX:
        return coherence_protocol->BusRdX(&lines[cache_id]);
    case BusUpdate:
        return coherence_protocol->BusUpdt(&lines[cache_id]);
    case BusUpgrade:
        return coherence_protocol->BusUpgr(&lines[cache_id]);
    case BusWrite:
        return coherence_protocol->BusWr(&lines[cache_id]);
    default: // Only respond to actual bus messages (enum has other values)
        return false;
    }
}

void TextbookModeCoherence::printCmdFormatMessage() {
    std::cerr << "Command must be 'E', 'R', or 'W' followed by a number between 1 and " << N_TEXTBOOK_LINES << ", or 'X'" << std::endl;
}

void TextbookModeCoherence::receiveEvict(uint32_t cache_id) {
    // Start a cache line eviction
    bus_events.clear();
    command = { Eviction, cache_id };

    /*
     * Code copied and adapted from Cache::allocate
     * Main changes are the removal of replacement policy involvement (replacement done manually)
     */

     // Evict the line first if necessary
    if (lines[cache_id].tag && coherence_protocol->isWriteBackNeeded(lines[cache_id].state)) {
        bus_events.emplace_back(LineFlush, cache_id);
        bus_events.emplace_back(WriteBack, cache_id);
    }

    // Initialize the line
    lines[cache_id].tag = 0;
    lines[cache_id].state = I;
}
void TextbookModeCoherence::receivePrRd(uint32_t cache_id) {
    // Start a processor write operation
    bus_events.clear();
    command = { ProcRead, cache_id };

    /*
     * Code copied and adapted from Cache::receivePrRd
     * Main changes are the removal of all statistics
     *   and replacement policy involvement (replacement done manually)
     */

     // Intercept read miss
    if (!lines[cache_id].tag) {
        lines[cache_id].tag = ALLOCATED;
        lines[cache_id].state = I;
    }

    // Initiate the PrRd state change
    coherence_protocol->PrRd(0, &lines[cache_id]);
}
void TextbookModeCoherence::receivePrWr(uint32_t cache_id) {
    // Start a processor write operation
    bus_events.clear();
    command = { ProcWrite, cache_id };

    /*
     * Code copied and adapted from Cache::receivePrWr
     * Main changes are the removal of all statistics except WriteMemory
     *   and replacement policy involvement (replacement done manually)
     */

     // Intercept write miss
    if (coherence_protocol->doesWriteNoAllocate())
        bus_events.emplace_back(WriteMemory, cache_id);
    else if (!lines[cache_id].tag) {
        lines[cache_id].tag = ALLOCATED;
        lines[cache_id].state = I;
    }

    // Initiate the PrWr state change
    coherence_protocol->PrWr(0, lines[cache_id].tag ? &lines[cache_id] : nullptr);
}

void TextbookModeCoherence::reset() {
    // Reset attributes
    command.event = Invalidation;
    command.issuer = 0;
    delete coherence_protocol;
    coherence_protocol = (*coherence_map)[p_name](*this);

    // Reset cache lines
    for (uint32_t i = 0; i < N_TEXTBOOK_LINES; i++) {
        lines[i].tag = 0;
        lines[i].state = I;
    }

    // Restart table
    printSeparator();
    printStats();
}

void TextbookModeCoherence::printSeparator() {
    // Fill each column with dashes, the separator is a plus ('+')
    for (uint32_t i = 0; i < COL_WIDTH_OP; i++)
        std::cout << '-';
    std::cout << "-+-";
    for (uint32_t i = 0; i < COL_WIDTH_EVENT; i++)
        std::cout << '-';
    std::cout << "-+-";
    for (uint32_t i = 0; i < COL_WIDTH_SOURCE; i++)
        std::cout << '-';
    std::cout << "-+-";
    for (uint32_t i = 0; i < COL_WIDTH_STATES; i++)
        std::cout << '-';
    std::cout << std::endl;

    // Clear the command and bus events
    bus_events.clear();
    command = { Invalidation, 0 };  // Here Invalidation is used to represent the lack of command
}
void TextbookModeCoherence::printStats() {
    // Print the command in the first column
    switch (command.event) {
    case Eviction:
        std::cout << 'E' << command.issuer + 1;
        break;
    case ProcRead:
        std::cout << 'R' << command.issuer + 1;
        break;
    case ProcWrite:
        std::cout << 'W' << command.issuer + 1;
        break;
    default:
        std::cout << std::setw(COL_WIDTH_OP) << ' ';
        break;
    }

    // Print the bus events; messages in the second column, source in the third
    // Bus events are printed with a separator, and the first one has a different format,
    //   so print the first one before the loop
    auto it = bus_events.begin();
    if (it == bus_events.end()) {
        // There are no bus events
        std::cout << " | " << std::setw(COL_WIDTH_EVENT) << ' ' << " | " << std::setw(COL_WIDTH_SOURCE) << ' ';
    } else {
        // There are bus events, print the first one
        std::cout << " | " << std::setw(COL_WIDTH_EVENT) << bus_event_names[it->event] << " | ";
        std::cout << std::setw(COL_WIDTH_SOURCE) << (it->issuer == N_TEXTBOOK_LINES ? "Main Memory" : "");
        it++;
    }
    // Print each subsequent bus event
    for (; it != bus_events.end(); it++) {
        // Print blank fourth column and first column between bus events
        std::cout << " |" << std::endl << std::setw(COL_WIDTH_OP) << ' ' << " |   ";
        // Print the bus message in the second column
        std::cout << std::setw(COL_WIDTH_EVENT - 2) << bus_event_names[it->event] << " | ";
        // Print the source in the third column
        if (it->issuer == N_TEXTBOOK_LINES)
            std::cout << std::setw(COL_WIDTH_SOURCE) << "Main Memory";
        else if (it->issuer != command.issuer)
            std::cout << 'P' << std::setw(COL_WIDTH_SOURCE - 1) << it->issuer + 1;
        else std::cout << std::setw(COL_WIDTH_SOURCE) << ' ';
    }

    // Print resulting cache line states
    std::cout << " |";
    for (uint32_t i = 0; i < N_TEXTBOOK_LINES; i++)
        std::cout << (lines[i].tag ? state_names[lines[i].state] : " - ");
    std::cout << std::endl;
}
