/// @file run_modes.cc
/// @brief Implementation of the three modes of operation: Single Metrics, Batch Metrics, and Textbook

#include <barrier>
#include <csignal>
#include <fstream>
#include <mutex>
#include <thread>

#include "main.h"
#include "memory_system.h"
#include "textbook_mode_coherence.h"
#include "textbook_mode_replacer.h"

/// @brief The number of traces to buffer at a time
#define N_TRACE_BUF 1000000

void runBatchMetrics(int argc, char* argv[]) {
    // Configurations vector
    std::vector<cache_config> configs;
    readConfigurations(configs, argv[ARG_CONFIG]);

    // Get trace file, trace limit, and trace file buffers
    std::ifstream trace_file;
    size_t trace_limit = getTrace(argc, argv, trace_file, ARG_M_COUNT);

    // The trace file chunks will be "double buffered" to allow for simultaneous reading and processing
    trace_t* trace_swap = new trace_t[N_TRACE_BUF];
    trace_t* trace_buf = new trace_t[N_TRACE_BUF];
    uint32_t bytes_read;

    // Setup synchronization objects
    auto sync_point_task = [&]() {
        // Switch to the new chunk
        std::swap(trace_buf, trace_swap);
        bytes_read = trace_file.gcount();
        };
    std::barrier sync_point(configs.size() + 1, sync_point_task);
    std::mutex stats_print_mutex;

    // Set up worker threads
    auto batch_metrics_task = [&](cache_config config) {
        // Create memory system
        MemorySystem* memory_system = (*directory_map)[config.directory](config);

        // Process each block as it arrives
        size_t line_count = 0;
        while (bytes_read) {
            uint32_t trace_count = bytes_read / 5;
            // Execute traces in current block
            for (uint32_t i = 0; i < trace_count; i++) {
                uint8_t op = trace_buf[i].op;
                addr_t addr = trace_buf[i].addr;
                if (op & 1) memory_system->issuePrWr(le32toh(addr), op >> 1
#ifdef WRITE_TIMESTAMP
                    , line_count
#endif
                );
                else memory_system->issuePrRd(le32toh(addr), op >> 1
#ifdef WRITE_TIMESTAMP
                    , line_count
#endif
                );
                line_count++;

                // Exit the while loop if trace limit is reached
                if (trace_limit && line_count == trace_limit) goto print_stats;
            }

            // Wait for the next block to be read in
            sync_point.arrive_and_wait();
        }

        // Print statistics (Note the mutex is automatically released as part of the implicit destructor call)
    print_stats:
        std::lock_guard guard(stats_print_mutex);
        memory_system->printStats();
        };
    std::vector<std::thread> workers;
    workers.reserve(configs.size());

    // Read first chunk
    trace_file.read((char*)trace_buf, N_TRACE_BUF * sizeof(trace_t));
    bytes_read = trace_file.gcount();

    // Start each worker thread
    printStatsHeader(); // Ensure CSV header prints first
    for (cache_config& config : configs)
        workers.emplace_back(batch_metrics_task, config);

    // Read each subsequent chunk while the worker threads process the current one
    size_t line_count = bytes_read / 5;
    while (bytes_read && !(trace_limit && line_count >= trace_limit)) {
        trace_file.read((char*)trace_swap, N_TRACE_BUF * sizeof(trace_t));
        sync_point.arrive_and_wait();
        line_count += bytes_read / 5;
    }

    // Wait for worker threads
    for (std::thread& worker : workers)
        worker.join();

    // Cleanup
    delete[] trace_buf;
    delete[] trace_swap;
}

void runSingleMetrics(int argc, char* argv[]) {
    // Get configuration
    cache_config config = { 0 };
    getConfig(argc, argv, config);

    // Get trace file and limit
    std::ifstream trace_file;
    size_t trace_limit = getTrace(argc, argv, trace_file, ARG_S_COUNT);

    // Create memory system
    MemorySystem* memory_system = (*directory_map)[config.directory](config);

    // Execute traces
    addr_t addr;
    uint8_t op;
    for (size_t line_count = 0; !(trace_file.eof() || (trace_limit && line_count == trace_limit)); line_count++) {
        trace_file.read((char*)&op, sizeof(op));
        trace_file.read((char*)&addr, sizeof(addr));
        if (op & 1) memory_system->issuePrWr(le32toh(addr), op >> 1
#ifdef WRITE_TIMESTAMP
            , line_count
#endif
        );
        else memory_system->issuePrRd(le32toh(addr), op >> 1
#ifdef WRITE_TIMESTAMP
            , line_count
#endif
        );
    }

    // Print statistics
    printStatsHeader();
    memory_system->printStats();
}

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
    signal(SIGINT, [](int signum) {
        fclose(stdin);
        std::cout << "\r";
        });

    // Process commands
    std::string line;
    for (uint32_t line_count = 1; std::getline(std::cin, line); line_count++) {
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
