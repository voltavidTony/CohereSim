/// @file main.cc
/// @brief This file processes the command line arguments and controls the program flow

#include <fstream>
#include <signal.h>

#include "memory_bus.h"
#include "textbook_mode_coherence.h"
#include "textbook_mode_replacer.h"

/// @brief The value of 'argc' if no arguments were passed on the command line
#define NO_ARGS 1

/// @brief Argument indices for single config run and within config file
enum args_single_e {
    ARG_S_PROG,
    ARG_CACHE_SIZE,
    ARG_LINE_SIZE,
    ARG_ASSOCIATIVITY,
    ARG_COHERENCE,
    ARG_REPLACEMENT,
    ARG_S_TRACE_FILE,
    ARG_S_TRACE_LIMIT,
    ARG_S_COUNT
};

/// @brief Argument indices for multiple configs file run
enum args_batch_e {
    ARG_M_PROG,
    ARG_CONFIG,
    ARG_M_TRACE_FILE,
    ARG_M_TRACE_LIMIT,
    ARG_M_COUNT
};

/// @brief Argument indices for textbook (interactive) mode
enum args_textbook_e {
    ARG_T_PROG,
    ARG_TEXTBOOK,
    ARG_T_COUNT
};

std::map<std::string, coh_factory_t, ci_less>* coherence_map = nullptr;
std::map<std::string, rep_factory_t, ci_less>* replacement_map = nullptr;

/// @brief CSV-friendly names for cache runtime statistics. Make sure these match up with 'bus_msg_e' and 'statistic_e'
constexpr const char* stat_names[N_STATISTICS] = {
    "processor reads", "processor writes",
    "bus reads", "bus readxs", "bus updates", "bus upgrades", "bus writes",
    "read misses", "write misses",
    "line flushes", "line fetches", "c2c transfers", "write backs", "memory writes",
    "evictions",
    "exclusions", "interventions", "invalidations"
};

/// @brief Provide error message and exit code on condition
/// @param condition Whether the program should print an error message and exit
/// @param msg The error message to print
/// @param config_id The ID of the config that caused the error
/// @param arg_index The argument of the config that caused the error
/// @see @ref docs/pages/exit_codes.md
void exitIf(bool condition, std::string msg, uint32_t config_id, uint32_t arg_index) {
    if (condition) {
        std::cerr << arg_index << '@' << config_id << ": " << msg << std::endl;
        exit((config_id << 3) + arg_index);
    }
}

/// @brief When Ctrl+C is issued, just close the input stream so the program can perform proper shutdown
/// @param signum The signal number
void signalCallback(int signum) {
    fclose(stdin);
    std::cout << "\r";
}

/// @brief Run the program in textbook mode (aka interactive mode)
/// @param name_of_showcased The selected policy/protocol
void runTextbookMode(char* name_of_showcased) {
    // Get the correct textbook mode class
    TextbookMode* textbook_mode;
    if (coherence_map->count(name_of_showcased)) textbook_mode = new TextbookModeCoherence(name_of_showcased);
    else if (replacement_map->count(name_of_showcased)) textbook_mode = new TextbookModeReplacer(name_of_showcased);
    else {
        std::cerr << ARG_TEXTBOOK << '@' << 0 << ": " << "Couldn't find a coherence protocol or replacement policy with that name!" << std::endl;
        exit(ARG_TEXTBOOK);
    }

    // Setup sigint catch (so that bottom border of table can be printed)
    signal(SIGINT, signalCallback);

    // Process commands
    std::string line;
    for (size_t line_count = 1; std::getline(std::cin, line); line_count++) {
        // Move cursor up one line if the output and input are the terminal
        if (isatty(fileno(stdin)) && isatty(fileno(stdout))) std::cout << "\e[A";

        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Evaluate command
        if (!textbook_mode->evalutateCommand(line)) {
            // Bad command
            if (!isatty(fileno(stdin))) std::cerr << "Line " << line_count << ": ";
            textbook_mode->printCmdFormatMessage();
        }
    }

    // Cleanup
    delete textbook_mode;
}

/// @brief Parse the given arguments into a memory bus configuration
/// @param argc The number of program arguments
/// @param argv The array of program arguments
/// @param config The configuration struct to populate
void getConfig(int argc, char* argv[], cache_config& config) {
    char* suffix;   // Points to the next character after each parse

    // Cache size
    config.cache_size = strtoul(argv[ARG_CACHE_SIZE], &suffix, 10);
    exitIf(suffix == argv[ARG_CACHE_SIZE], "Invalid format for cache size (expect positive number of bytes)", config.id, ARG_CACHE_SIZE);
    exitIf(config.cache_size == 0 || config.cache_size & (config.cache_size - 1), "Cache size must be a power of 2", config.id, ARG_CACHE_SIZE);
    switch (suffix[0]) {
    case '\0':
        break;
    case 'M':
        config.cache_size *= 1024;
    case 'k':
        config.cache_size *= 1024;
        if (suffix[1] == '\0') break;
    default:
        exitIf(true, "Invalid cache size unit (expect either 'k' or 'M')", config.id, ARG_CACHE_SIZE);
    }

    // Line size
    config.line_size = strtoul(argv[ARG_LINE_SIZE], &suffix, 10);
    exitIf(suffix == argv[ARG_LINE_SIZE] || *suffix, "Invalid format for line size (expect positive integer)", config.id, ARG_LINE_SIZE);
    exitIf(config.line_size == 0 || config.line_size & (config.line_size - 1), "Line size must be a power of 2", config.id, ARG_LINE_SIZE);
    exitIf(config.line_size > config.cache_size, "Line size cannot exceed the cache size", config.id, ARG_LINE_SIZE);

    // Associativity
    config.assoc = strtoul(argv[ARG_ASSOCIATIVITY], &suffix, 10);
    exitIf(suffix == argv[ARG_ASSOCIATIVITY] || *suffix, "Invalid format for associativity (expect positive integer)", config.id, ARG_ASSOCIATIVITY);
    exitIf(config.assoc == 0 || config.assoc & (config.assoc - 1), "Associativity must be a power of 2", config.id, ARG_ASSOCIATIVITY);
    exitIf(config.assoc * config.line_size > config.cache_size, "Associativity cannot exceed the number of lines", config.id, ARG_ASSOCIATIVITY);

    // Coherence protocol
    exitIf(!coherence_map->count(argv[ARG_COHERENCE]), "Coherence protocol not found", config.id, ARG_COHERENCE);
    config.coherence = argv[ARG_COHERENCE];

    // Replacement policy
    exitIf(!replacement_map->count(argv[ARG_REPLACEMENT]), "Replacement policy not found", config.id, ARG_REPLACEMENT);
    config.replacer = argv[ARG_REPLACEMENT];
}

/// @brief Open the trace file and read the trace limit
/// @param argc The number of program arguments
/// @param argv The array of program arguments
/// @param trace_file An ifstream that will become the trace file
/// @param arg_max_count The number of arguments when the trace limit argument is present
/// @return The trace limit
size_t getTrace(int argc, char* argv[], std::ifstream& trace_file, int arg_max_count) {
    // Open trace file (2nd to last argument)
    trace_file.open(argv[arg_max_count - 2], std::ios_base::in | std::ios_base::binary);
    std::string tf_error = "Trace file read error: ";
    exitIf(!trace_file, tf_error + std::strerror(errno), 0, arg_max_count - 2);
    if (argc < arg_max_count) return 0;

    // Get trace limit (last argument)
    char* suffix;
    size_t trace_limit = strtoull(argv[arg_max_count - 1], &suffix, 10);
    exitIf(suffix == argv[arg_max_count - 1] || *suffix, "Invalid format for trace limit (expect positive integer)", 0, arg_max_count - 1);
    return trace_limit;
}

/// @brief Run the program in batch mode
/// @param argc The number of command line arguments
/// @param argv An array to the command line arguments
void runBatchMetrics(int argc, char* argv[]) {
    std::cerr << "Batch metrics mode is not yet implemented!" << std::endl;
}

/// @brief Process a trace file from a single config run (config from cmd args)
/// @param argc The number of program arguments
/// @param argv The array of program arguments
void runSingleMetrics(int argc, char* argv[]) {
    // Get configuration
    cache_config config = { 0 };
    getConfig(argc, argv, config);

    // Get trace file and limit
    std::ifstream trace_file;
    size_t trace_limit = getTrace(argc, argv, trace_file, ARG_S_COUNT);

    // Create memory bus
    MemoryBus memory_bus(config);

    // Execute traces
    addr_t addr;
    uint8_t op;
    for (size_t line_count = 0; !(trace_file.eof() || (trace_limit && line_count == trace_limit)); line_count++) {
        trace_file.read((char*)&op, sizeof(op));
        trace_file.read((char*)&addr, sizeof(addr));
        le32toh(addr);
        if (op & 1) memory_bus.issuePrWr(addr, op >> 1);
        else memory_bus.issuePrRd(addr, op >> 1);
    }

    // Print statistics
    std::cout << "config,core,miss rate";
    for (size_t i = 0; i < N_STATISTICS; i++) std::cout << ',' << stat_names[i];
    std::cout << std::endl;
    memory_bus.printStats();
}

/// @brief Print the program usage method
void usageMsg() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  (1) ./simulate_cache <coherence|replacer>" << std::endl;
    std::cout << "  (2) ./simulate_cache <configuration> <trace_file> [trace_limit]" << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  (1) Run the simulator in textbook mode (see the manual for more info)" << std::endl;
    std::cout << "  (2) Run the simulator in standard mode (see below)" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  configuration: Either a single memory bus configuration (see below) or" << std::endl;
    std::cout << "                   the path to a file containing multiple memory bus configurations" << std::endl;
    std::cout << "  trace_file:    The path to the input trace file" << std::endl;
    std::cout << "  trace_limit:   (Optional) The maximum number of trace entries to read" << std::endl;
    std::cout << "Memory bus configuration:" << std::endl;
    std::cout << "  Syntax:" << std::endl;
    std::cout << "    <cache_size[unit]> <line_size> <associativity> <coherence> <replacer>" << std::endl;
    std::cout << "  Options:" << std::endl;
    std::cout << "    associativity: The associativity of the cache" << std::endl;
    std::cout << "    cache_size:    The size of the cache in bytes or in the specified unit" << std::endl;
    std::cout << "    coherence:     The name of the coherence protocol (not case sensitive). One of:" << std::endl;
    for (auto& [coh, factory] : *coherence_map) std::cout << "                     - " << coh << std::endl;
    std::cout << "    line_size:     The size of a line in the cache" << std::endl;
    std::cout << "    replacer:      The name of the replacement policy (not case sensitive). One of:" << std::endl;
    for (auto& [rep, factory] : *replacement_map) std::cout << "                     - " << rep << std::endl;
    std::cout << "    unit:          (Optional) The unit of the cache size." << std::endl;
    std::cout << "                     Either 'k' or 'M' for kilobytes and megabytes respectively" << std::endl;
}

/// @brief The main function decides which mode to execute based on the number of arguments supplied
/// @param argc The number of command line arguments
/// @param argv An array to the command line arguments
/// @return The program exit code 
/// @see @ref docs/pages/exit_codes.md
int main(int argc, char* argv[]) {
    switch (argc) {
    case NO_ARGS:
        usageMsg();
        return 0;
    case ARG_T_COUNT:
        runTextbookMode(argv[ARG_TEXTBOOK]);
        return 0;
    case ARG_M_COUNT - 1:
    case ARG_M_COUNT:
        runBatchMetrics(argc, argv);
        return 0;
    case ARG_S_COUNT - 1:
    case ARG_S_COUNT:
        runSingleMetrics(argc, argv);
        return 0;
    default:
        std::cerr << "Argument count mismatch" << std::endl;
        return -1;
    }
}

bool ci_less::operator()(const std::string& s1, const std::string& s2) const {
    std::string s1l = s1, s2l = s2;
    for (char& c : s1l) c = std::tolower(c);
    for (char& c : s2l) c = std::tolower(c);
    return s1l < s2l;
}
