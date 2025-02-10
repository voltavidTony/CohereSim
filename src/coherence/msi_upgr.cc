/// @file msi_upgr.cc
/// @brief Implementation of the MSIUpgr coherence protocol

#include "msi_upgr.h"

ADD_COHERENCE_TO_CMD_LINE(MSIUpgr);

void MSIUpgr::PrRd(cache_line* line) {
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

void MSIUpgr::PrWr(cache_line* line) {
    switch (line->state) {
    case I:
        cache.issueBusMsg(BusReadX);
        line->state = M;
        break;
    case S:
        cache.issueBusMsg(BusUpgrade);
        line->state = M;
    case M:
        break;
    default:
        STATE_ERR;
        return;
    }
}

bool MSIUpgr::BusRd(cache_line* line) {
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

bool MSIUpgr::BusRdX(cache_line* line) {
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

bool MSIUpgr::BusUpgr(cache_line* line) {
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

bool MSIUpgr::isWriteBackNeeded(state_e state) {
    return state == M;
}
