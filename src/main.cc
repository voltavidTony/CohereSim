/// @file main.cc
/// @brief This file processes the command line arguments, provides helper methods for processing the command line arguments, and decides what to run

#include <filesystem>
#include <fstream>

#include "typedefs.h"
#include "run_modes.h"

/// @brief The value of 'argc' if no arguments were passed on the command line
#define NO_ARGS 1

/// @brief The number of columns that appear in the output statistics
#define NUM_COLUMNS (N_STATISTICS + 3)

/// @brief The size of the config line buffer
#define CONFIG_LINE_SIZE (ARG_C_COUNT * 10)

std::map<std::string, coh_factory_t, ci_less>* coherence_map = nullptr;
std::map<std::string, dir_factory_t, ci_less>* directory_map = nullptr;
std::map<std::string, rep_factory_t, ci_less>* replacement_map = nullptr;

/// @brief CSV-friendly names for cache runtime statistics. Make sure these match up with 'bus_msg_e' and 'statistic_e'
constexpr const char* stat_names[NUM_COLUMNS] = {
    "config", "core", "miss rate",
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

    // Directory protocol
    exitIf(!directory_map->count(argv[ARG_DIRECTORY]), "Directory protocol not found", config.id, ARG_DIRECTORY);
    config.directory = argv[ARG_DIRECTORY];
}

size_t getTrace(int argc, char* argv[], std::ifstream& trace_file, int arg_max_count) {
    // Open trace file (2nd to last argument)
    trace_file.open(argv[arg_max_count - 2], std::ios_base::in | std::ios_base::binary);
    std::string tf_error = "Trace file read error: ";
    exitIf(!trace_file, tf_error + std::strerror(errno), 0, arg_max_count - 2);
    exitIf(std::filesystem::file_size(argv[arg_max_count - 2]) % sizeof(trace_t), "Malformed trace file", 0, arg_max_count - 2);

    // If trace limit was not specified
    if (argc < arg_max_count) return 0;

    // Get trace limit (last argument)
    char* suffix;
    size_t trace_limit = strtoull(argv[arg_max_count - 1], &suffix, 10);
    exitIf(suffix == argv[arg_max_count - 1] || *suffix, "Invalid format for trace limit (expect positive integer)", 0, arg_max_count - 1);
    return trace_limit;
}

void readConfigurations(std::vector<cache_config>& configs, char* configs_file_path) {
    // Get the configs file
    std::ifstream configs_file;
    configs_file.open(configs_file_path);
    std::string cf_error = "Configs file read error: ";
    exitIf(!configs_file, cf_error + std::strerror(errno), 0, ARG_CONFIG);

    // Need to simulate argv like from the command line when reading in the configurations
    char** config_argv = new char* [ARG_C_COUNT];
    char* config_line_cstr = new char[CONFIG_LINE_SIZE] {};
    config_argv[0] = nullptr; // First argument is unused
    config_argv[1] = config_line_cstr; // Second argument is the start of the c-string

    // Read in configurations
    std::string config_line;
    for (uint32_t config_id = 1; std::getline(configs_file, config_line); config_id++) {
        // A space-separated string can be turned into an argv string by replacing the spaces with nulls
        //   (as long as there is no quotation or other space-preserving stuff, which is true in this case)
        uint32_t config_argc = 2;
        uint32_t copy_i = 0;
        while (copy_i < CONFIG_LINE_SIZE && config_line[copy_i]) {
            config_line_cstr[copy_i] = config_line[copy_i];
            if (config_line_cstr[copy_i] == ' ') {
                exitIf(config_argc == ARG_C_COUNT, "Too many arguments in cache config", config_id, ARG_C_COUNT);
                config_line_cstr[copy_i] = '\0';
                config_argv[config_argc++] = &config_line_cstr[copy_i + 1];
            }
            copy_i++;
        }
        exitIf(config_argc < ARG_C_COUNT, "Too few arguments in cache config", config_id, ARG_C_COUNT);
        exitIf(copy_i == CONFIG_LINE_SIZE, "Config string longer than expected", config_id, ARG_C_COUNT);
        config_line_cstr[copy_i] = '\0';

        // Parse the configuration
        cache_config config;
        config.id = config_id;
        getConfig(config_argc, config_argv, config);
        configs.emplace_back(config); // Inefficient, but inconsequential in the context of this program
    }

    // Cleanup
    delete config_argv;
    delete config_line_cstr;
}

void printStatsHeader() {
    std::cout << stat_names[0];
    for (uint32_t i = 1; i < NUM_COLUMNS; i++) std::cout << ',' << stat_names[i];
    std::cout << std::endl;
}

/// @brief Print the program usage method
void usageMsg() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  (1) ./simulate_cache <coherence|replacer>" << std::endl;
    std::cout << "  (2) ./simulate_cache <configuration> <trace_file> [trace_limit]" << std::endl;
    std::cout << "Description:" << std::endl;
    std::cout << "  (1) Run the simulator in textbook mode (see the manual for more info)" << std::endl;
    std::cout << "  (2) Run the simulator in metrics mode (see below)" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  configuration: Either a single memory system configuration (see below) or" << std::endl;
    std::cout << "                   the path to a file containing multiple memory system configurations" << std::endl;
    std::cout << "  trace_file:    The path to the input trace file" << std::endl;
    std::cout << "  trace_limit:   (Optional) The maximum number of trace entries to read" << std::endl;
    std::cout << "Memory system configuration:" << std::endl;
    std::cout << "  Syntax:" << std::endl;
    std::cout << "    <cache_size[unit]> <line_size> <associativity> <coherence> <replacer> <directory>" << std::endl;
    std::cout << "  Options:" << std::endl;
    std::cout << "    associativity: The associativity of the cache" << std::endl;
    std::cout << "    cache_size:    The size of the cache in bytes or in the specified unit" << std::endl;
    std::cout << "    coherence:     The name of the coherence protocol (not case sensitive). One of:" << std::endl;
    for (auto& [coh, factory] : *coherence_map) std::cout << "                     - " << coh << std::endl;
    std::cout << "    directory:     The name of the directory protocol (not case sensitive). One of:" << std::endl;
    for (auto& [coh, factory] : *directory_map) std::cout << "                     - " << coh << std::endl;
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
