/// @file write_through.cc
/// @brief Implementation of the WriteThrough coherence protocol

#include "write_through.h"

ADD_COHERENCE_TO_CMD_LINE(WriteThrough);

void WriteThrough::PrRd(cache_line* line) {
    switch (line->state) {
    case V:
        break;
    case I:
        cache.issueBusMsg(BusRead);
        line->state = V;
        break;
    default:
        STATE_ERR;
        return;
    }
}

void WriteThrough::PrWr(cache_line* line) {
    if (line) switch (line->state) {
    case V:
    case I:
        cache.issueBusMsg(BusWrite);
        break;
    default:
        STATE_ERR;
        return;
    } else {
        cache.issueBusMsg(BusWrite);
    }
}

bool WriteThrough::BusRd(cache_line* line) {
    switch (line->state) {
    case V:
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool WriteThrough::BusWr(cache_line* line) {
    switch (line->state) {
    case V:
        line->state = I;
    case I:
        return false;
    default:
        STATE_ERR;
        return false;
    }
}

bool WriteThrough::doesWriteNoAllocate() {
    return true;
}

bool WriteThrough::isWriteBackNeeded(state_e state) {
    return false;
}
