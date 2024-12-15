/// @file dragon.cc
/// @brief Implementation of the Dragon coherence protocol

#include "dragon.h"

ADD_COHERENCE_TO_CMD_LINE(Dragon);

void Dragon::PrRd(addr_t addr, cache_line* line) {
    switch (line->state) {
    case E:
    case Sc:
    case Sm:
    case M:
        break;
    case Unallocated:
        line->state = cache.issueBusMsg(BusRead, addr) ? Sc : E;
        break;
    default:
        STATE_ERR;
        return;
    }
}

void Dragon::PrWr(addr_t addr, cache_line* line) {
    switch (line->state) {
    case E:
        line->state = M;
        break;
    case Sc:
    case Sm:
        line->state = cache.issueBusMsg(BusUpdate, addr) ? Sm : M;
    case M:
        break;
    case Unallocated:
        line->state =
            // Short circuit AND. BusUpdate is only issued if other caches have the line
            cache.issueBusMsg(BusRead, addr) && cache.issueBusMsg(BusUpdate, addr)
            ? Sm : M;
        break;
    default:
        STATE_ERR;
        return;
    }
}

bool Dragon::BusRd(cache_line* line) {
    switch (line->state) {
    case E:
        line->state = Sc;
    case Sc:
        return false;
    case M:
        line->state = Sm;
    case Sm:
        return true;
    default:
        STATE_ERR;
        return false;
    }
}

bool Dragon::BusUpdt(cache_line* line) {
    switch (line->state) {
    case Sm:
        line->state = Sc;
    case Sc:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool Dragon::doesDirtySharing() {
    return true;
}

bool Dragon::isWriteBackNeeded(state_e state) {
    return state == Sm || state == M;
}
