/// @file extractor.c
/// @brief This program will intercept the output of gem5 and redirect the traces to a binary file

#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef NCPU
/// @brief The number of CPU cores in the current gem5 run (defined externally)
#define NCPU 16
#endif

/// @brief Like fprintf, but specifically for integers and with the standard thousands separator
/// @param file The file to print to
/// @param n The number to print to the file
/// @param width The character width of the number, like the n in %nd
void fprintfcomma(FILE* file, uint64_t n, int width) {
    if (n < 1000) { // Base case
        char buf[10];
        sprintf(buf, "  %%%dld", width);
        fprintf(file, buf, n);
    } else { // Recursive case
        fprintfcomma(file, n / 1000, width - 4);
        fprintf(file, ",%03ld", n % 1000);
    }
}

/// @brief The main function intercepts the output of gem5, saving the memory traces to a binary file and memory operation statistics to a stat file
/// @param argc The number of command line arguments
/// @param argv An array to the ocmmand line arguments
/// @return The program exit code
int main(int argc, char const* argv[]) {
    // Verify args
    if (argc < 2) {
        printf("Please specify a path to save the trace binary! (./extractor [tracefile])\n");
        return -1;
    }

    // Open trace file
    char buf[BUFSIZ];
    sprintf(buf, "%s.bin", argv[1]);
    FILE* trace = fopen(buf, "wb");
    if (trace == NULL) {
        printf("Couldn't open trace file for writing: %s", strerror(errno));
        return errno;
    }

    // Open trace statistics file
    sprintf(buf, "%s.stat", argv[1]);
    FILE* tracestat = fopen(buf, "w");
    if (tracestat == NULL) {
        printf("Couldn't open trace statistics file for writing: %s", strerror(errno));
        return errno;
    }

    // Get start time
    time_t tracetime = time(NULL);

    // Read simulator output from stdin
    uint64_t ifetch[NCPU] = { 0 };
    uint64_t read[NCPU] = { 0 };
    uint64_t write[NCPU] = { 0 };
    while (fgets(buf, sizeof(char) * BUFSIZ, stdin)) {
        int cpu = 0;
        char op = 0;
        int addr = 0;

        // Skip non-trace lines
        if (sscanf(buf, "%d\t%c\t%x\n", &cpu, &op, &addr) != 3) {
            printf("%s", buf);
            continue;
        }

        // Record stats
        switch (op) {
        case 'i':
            ifetch[cpu]++;
            continue; // Don't write instruction fetches to trace file
        case 'r':
            read[cpu]++;
            break;
        case 'w':
            write[cpu]++;
            break;
        default: // Trace operation is invalid
            printf("%s", buf);
            continue;
        }

        // Write trace to binary file (7 bits for CPU core id, 1 bit for operation, and 4 bytes for memory address)
        fputc((cpu << 1) | (op == 'w'), trace);
        addr = htole32(addr);
        fwrite(&addr, sizeof(addr), 1, trace);
    }

    // Get total trace generation time
    tracetime = time(NULL) - tracetime;
    int s = tracetime % 60;
    int m = (tracetime / 60) % 60;
    int h = tracetime / 3600;

    // Write trace generation time to file
    fprintf(tracestat, "Time to generate trace file: ");
    if (h) fprintf(tracestat, "%dh %dm %ds\n\n", h, m, s);
    else if (m) fprintf(tracestat, "%dm %ds\n\n", m, s);
    else fprintf(tracestat, "%ds\n\n", s);

    // Get column widths
    int cols[NCPU] = { 0 };
    for (size_t i = 0; i < NCPU; i++) {
        int num = ifetch[i] > i ? ifetch[i] : i;
        do cols[i]++;
        while (num /= 10);
        cols[i] += (cols[i] - 1) / 3;
    }

    // Write trace statistics to file
    fprintf(tracestat, "CPU:     ");
    for (size_t i = 0; i < NCPU; i++) fprintfcomma(tracestat, i, cols[i]);
    fprintf(tracestat, "\nIFetches:");
    for (size_t i = 0; i < NCPU; i++) fprintfcomma(tracestat, ifetch[i], cols[i]);
    fprintf(tracestat, "\nReads:   ");
    for (size_t i = 0; i < NCPU; i++) fprintfcomma(tracestat, read[i], cols[i]);
    fprintf(tracestat, "\nWrites:  ");
    for (size_t i = 0; i < NCPU; i++) fprintfcomma(tracestat, write[i], cols[i]);
    fprintf(tracestat, "\n");

    // Cleanup
    fclose(trace);
    fclose(tracestat);
    return 0;
}
