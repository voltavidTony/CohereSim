/// @file rr.cc
/// @brief Implementation of the random replacement policy

#include <cstdlib>

#include "rr.h"

ADD_REPLACER_TO_CMD_LINE(RR);

RR::RR(CacheABC& cache, uint32_t num_sets, uint32_t assoc)
    : ReplacementPolicy(cache, num_sets, assoc) {
    std::srand(num_sets * assoc);
}

uint32_t RR::getVictim(uint32_t set_idx) {
    return std::rand() % assoc;
}
