/// @file fifo.cc
/// @brief Implementation of the FIFO replacement policy

#include "fifo.h"

ADD_REPLACER_TO_CMD_LINE(FIFO);

FIFO::FIFO(CacheABC& cache, uint32_t num_sets, uint32_t assoc)
    : ReplacementPolicy(cache, num_sets, assoc) {
    up_next = new uint32_t[num_sets]{};
}
FIFO::~FIFO() {
    delete[] up_next;
}

uint32_t FIFO::getVictim(uint32_t set_idx) {
    uint32_t next = up_next[set_idx];
    up_next[set_idx] = (up_next[set_idx] + 1) % assoc;
    return next;
}

void FIFO::printState(uint32_t set_idx) {
    if (set_idx >= num_sets) return;
    uint32_t next = up_next[set_idx];
    std::cout << next;
    for (uint32_t i = 1; i < assoc; i++)
        std::cout << ' ' << (next + i) % assoc;
}
