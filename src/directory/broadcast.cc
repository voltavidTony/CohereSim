/// @file broadcast.cc
/// @brief Implementation of the Broadcast directory protocol

#include "broadcast.h"
#include "cache.h"

ADD_DIRECTORY_TO_CMD_LINE(Broadcast);

void Broadcast::issueBusMsg(bus_msg_e bus_msg, addr_t addr, uint32_t cache_id) {
    for (uint32_t i = 0; i < MAX_N_CACHES; i++)
        if (i != cache_id && caches[i])
            caches[i]->receiveBusMsg(bus_msg, addr);
}
