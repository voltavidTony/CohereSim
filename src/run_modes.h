/// @file run_modes.h
/// @brief Declaration of the run mode functions

#pragma once

/// @brief Run the program in batch mode
/// @param argc The number of command line arguments
/// @param argv An array to the command line arguments
void runBatchMetrics(int argc, char* argv[]);

/// @brief Process a trace file from a single config run (config from cmd args)
/// @param argc The number of program arguments
/// @param argv The array of program arguments
void runSingleMetrics(int argc, char* argv[]);

/// @brief Run the program in textbook mode (aka interactive mode)
/// @param name_of_showcased The selected policy/protocol
void runTextbookMode(char* name_of_showcased);
