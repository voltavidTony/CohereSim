/// @file main.h
/// @brief Declaration of the helper methods in main.cc

#pragma once

#include "typedefs.h"

/// @brief Parse the given arguments into a memory system configuration
/// @param argc The number of program arguments
/// @param argv The array of program arguments
/// @param config The configuration struct to populate
void getConfig(int argc, char* argv[], cache_config& config);

/// @brief Open the trace file and read the trace limit
/// @param argc The number of program arguments
/// @param argv The array of program arguments
/// @param trace_file An ifstream that will become the trace file
/// @param arg_max_count The number of arguments when the trace limit argument is present
/// @return The trace limit
size_t getTrace(int argc, char* argv[], std::ifstream& trace_file, int arg_max_count);

/// @brief Print the header row in the statistics output CSV
void printStatsHeader();

/// @brief Parse the cache configurations from the given configs file
/// @param configs The vector to contain the configurations
/// @param configs_file_path The file path to the configs file
void readConfigurations(std::vector<cache_config>& configs, char* configs_file_path);
