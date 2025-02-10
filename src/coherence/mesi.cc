/// @file mesi.cc
/// @brief Implementation of the MESI coherence protocol

#include "mesi.h"

ADD_COHERENCE_TO_CMD_LINE(MESI);

void MESI::PrRd(cache_line* line) {
    switch (line->state) {
    case M:
    case E:
    case S:
        break;
    case I:
        line->state = cache.issueBusMsg(BusRead) ? S : E;
        break;
    default:
        STATE_ERR;
        return;
    }
}

void MESI::PrWr(cache_line* line) {
    switch (line->state) {
    case I:
        cache.issueBusMsg(BusReadX);
        line->state = M;
        break;
    case S:
        cache.issueBusMsg(BusUpgrade);
    case E:
        line->state = M;
    case M:
        break;
    default:
        STATE_ERR;
        return;
    }
}

bool MESI::BusRd(cache_line* line) {
    switch (line->state) {
    case M:
    case E:
        line->state = S;
    case S:
        return true;
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool MESI::BusRdX(cache_line* line) {
    switch (line->state) {
    case M:
    case E:
    case S:
        line->state = I;
        return true;
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool MESI::BusUpgr(cache_line* line) {
    switch (line->state) {
    case S:
        line->state = I;
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool MESI::isWriteBackNeeded(state_e state) {
    return state == M;
}
