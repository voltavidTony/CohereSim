/// @file moesi.cc
/// @brief Implementation of the MOESI coherence protocol

#include "moesi.h"

ADD_COHERENCE_TO_CMD_LINE(MOESI);

void MOESI::PrRd(cache_line* line) {
    switch (line->state) {
    case M:
    case O:
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

void MOESI::PrWr(cache_line* line) {
    switch (line->state) {
    case I:
        cache.issueBusMsg(BusReadX);
        line->state = M;
        break;
    case O:
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

bool MOESI::BusRd(cache_line* line) {
    switch (line->state) {
    case M:
        line->state = O;
    case O:
        return true;
    case E:
        line->state = S;
        return true;
    case S:
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool MOESI::BusRdX(cache_line* line) {
    switch (line->state) {
    case M: // Textbook forgets to explain this state transition
    case O:
    case E:
        line->state = I;
        return true;
    case S:
        line->state = I;
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool MOESI::BusUpgr(cache_line* line) {
    switch (line->state) {
    case O: // This time O line is not flushed, so not sharer line
    case S:
        line->state = I;
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool MOESI::doesDirtySharing() {
    return true;
}

bool MOESI::isWriteBackNeeded(state_e state) {
    return state == M || state == O;
}
