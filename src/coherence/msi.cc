/// @file msi.cc
/// @brief Implementation of the MSI coherence protocol

#include "msi.h"

ADD_COHERENCE_TO_CMD_LINE(MSI);

void MSI::PrRd(cache_line* line) {
    switch (line->state) {
    case M:
    case S:
        break;
    case I:
        cache.issueBusMsg(BusRead);
        line->state = S;
        break;
    default:
        STATE_ERR;
        return;
    }
}

void MSI::PrWr(cache_line* line) {
    switch (line->state) {
    case I:
    case S:
        cache.issueBusMsg(BusReadX);
        line->state = M;
    case M:
        break;
    default:
        STATE_ERR;
        return;
    }
}

bool MSI::BusRd(cache_line* line) {
    switch (line->state) {
    case M:
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

bool MSI::BusRdX(cache_line* line) {
    switch (line->state) {
    case M:
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

bool MSI::isWriteBackNeeded(state_e state) {
    return state == M;
}
