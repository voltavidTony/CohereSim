/// @file cache.cc
/// @brief Definition of the Cache class methods

#include "cache.h"
#include "coherence_protocol.h"
#include "memory_bus.h"
#include "replacement_policy.h"

Cache::Cache(MemoryBus& memory_bus, bool& flushed, uint32_t cache_id, cache_config& config) :
    memory_bus(memory_bus), flushed(flushed), config(config), cache_id(cache_id) {
    // Calculate cache dimensions
    uint32_t num_lines = config.cache_size / config.line_size;
    num_sets = num_lines / config.assoc;
    line_offset = MSB(config.line_size);
    tag_offset = MSB(config.cache_size / config.assoc);

    // Initialize cache components
    coherence_protocol = (*coherence_map)[config.coherence](*this);
    replacement_policy = config.assoc == 1
        ? new ReplacementPolicy(*this, num_sets, config.assoc) // Proverbial "None" Replacer
        : (*replacement_map)[config.replacer](*this, num_sets, config.assoc);

    // Initialize cache lines
    lines = new cache_line[num_lines];
    for (uint32_t i = 0; i < num_lines; i++)
        lines[i] = (cache_line){ (tag_t)~0, I };
}
Cache::~Cache() {
    delete coherence_protocol;
    delete replacement_policy;
    delete[] lines;
}

void Cache::receivePrRd(addr_t addr) {
    // Remember the current address being accessed so that it can be attached to issued bus messages
    curr_access_addr = addr;

    // Find the accessed line
    cache_line* line = findLine(addr);
    statistics[ProcRead]++;

    // Intercept read miss
    if (!line) line = allocate(addr);
    if (!line->state) statistics[ReadMiss]++;

    // Initiate the PrRd state change
    state_e prev_state = line->state;
    coherence_protocol->PrRd(line);
    stateChangeStatistic(prev_state, line->state);

    // Inform replacer of cache line access
    uint32_t line_idx = line - lines;
    replacement_policy->touch(line_idx / config.assoc, line_idx % config.assoc);
}
void Cache::receivePrWr(addr_t addr) {
    // Remember the current address being accessed so that it can be attached to issued bus messages
    curr_access_addr = addr;

    // Find the accessed line
    cache_line* line = findLine(addr);
    statistics[ProcWrite]++;

    // Intercept write miss
    if (coherence_protocol->doesWriteNoAllocate()) {
        statistics[WriteMemory]++;
        if (!line || !line->state) statistics[WriteMiss]++;
    } else {
        if (!line) line = allocate(addr);
        if (!line->state) statistics[WriteMiss]++;
    }

    // Initiate the PrWr state change
    state_e prev_state;
    if (line) prev_state = line->state;
    coherence_protocol->PrWr(line);
    if (line) stateChangeStatistic(prev_state, line->state);

    // Inform replacer of cache line access
    if (line && line->state) {
        uint32_t line_idx = line - lines;
        replacement_policy->touch(line_idx / config.assoc, line_idx % config.assoc);
    }
}

bool Cache::issueBusMsg(bus_msg_e bus_msg) {
    // Send the bus message to each cache
    bool copies;
    switch (bus_msg) {
    case BusRead:
    case BusReadX:
        flushed = false;
        copies = memory_bus.issueBusMsg(bus_msg, curr_access_addr, cache_id);
        // Figure out where the cache line was read from
        statistics[flushed ? CacheToCache : LineFetch]++;
        break;
    case BusUpdate:
    case BusUpgrade:
    case BusWrite:
        copies = memory_bus.issueBusMsg(bus_msg, curr_access_addr, cache_id);
        break;
    default: // Only respond to actual bus messages (enum has other values)
        return false;
    }
    statistics[bus_msg]++;
    return copies;
}
bool Cache::receiveBusMsg(bus_msg_e bus_msg, addr_t addr) {
    // Find the accessed line
    cache_line* line = findLine(addr);
    if (!line) return false;

    // Map bus_msg_e to the appropriate function call, keeping track of the line's state and if the line was flushed
    state_e prev_state = line->state;
    switch (bus_msg) {
    case BusRead:
        if (coherence_protocol->BusRd(line)) {
            // The BusRead message requires extra logic for determining when a WriteBack occurs
            if (!coherence_protocol->doesDirtySharing() && coherence_protocol->isWriteBackNeeded(prev_state))
                statistics[WriteBack]++;
            statistics[LineFlush]++;
            flushed = true;
        }
        break;
    case BusReadX:
        if (coherence_protocol->BusRdX(line)) {
            statistics[LineFlush]++;
            flushed = true;
        }
        break;
    case BusUpdate:
        if (coherence_protocol->BusUpdt(line)) {
            statistics[LineFlush]++;
            flushed = true;
        }
        break;
    case BusUpgrade:
        if (coherence_protocol->BusUpgr(line)) {
            statistics[LineFlush]++;
            flushed = true;
        }
        break;
    case BusWrite:
        if (coherence_protocol->BusWr(line)) {
            statistics[LineFlush]++;
            flushed = true;
        }
        break;
    default: // Only respond to actual bus messages (enum has other values)
        return false;
    }
    stateChangeStatistic(prev_state, line->state);

    return line->state;
}

state_e Cache::getLineState(uint32_t set_idx, uint32_t way_idx) {
    return lines[set_idx * config.assoc + way_idx].state;
}

void Cache::printStats() {
    if (statistics[ProcRead] + statistics[ProcWrite]) {
        // Print miss rate
        std::cout << config.id << ',' << cache_id << ',' <<
            ((double)statistics[ReadMiss] + (double)statistics[WriteMiss]) / ((double)statistics[ProcRead] + (double)statistics[ProcWrite]);
        // Print each statistics value
        for (uint32_t i = 0; i < N_STATISTICS; i++) std::cout << ',' << statistics[i];
        std::cout << std::endl;
    }
}

void Cache::stateChangeStatistic(state_e before, state_e after) {
    // There are three state change statistics:
    //   - When a state transitions from a non-invalid state to the invalid state
    //   - When a state transitions from a shared state to a non-shared state
    //   - When a state transitions from a non-shared state to a shared state
    if (!before) return;
    else if (!after) statistics[Invalidation]++;
    else if (before <= V && after >= O) statistics[Intervention]++;
    else if (before >= O && after <= V) statistics[Exclusion]++;
}

cache_line* Cache::allocate(addr_t addr) {
    // Find the line index of the victim line
    // First, idx is the set index, and with the help of the replacer,
    //   is converted into line index
    uint32_t idx = ((addr >> line_offset) % num_sets);
    idx = replacement_policy->getVictim(idx) + idx * config.assoc;

    // Evict the line first if necessary
    if (lines[idx].state) {
        statistics[Eviction]++;
        if (coherence_protocol->isWriteBackNeeded(lines[idx].state)) {
            statistics[LineFlush]++;
            statistics[WriteBack]++;
        }
    }

    // Initialize the line
    lines[idx].tag = addr >> tag_offset;
    lines[idx].state = I;
    return &lines[idx];
}
cache_line* Cache::findLine(addr_t addr) {
    // Cache line tag
    tag_t tag = addr >> tag_offset;
    // Cache line index of the first line in the set
    uint32_t start_idx = ((addr >> line_offset) % num_sets) * config.assoc;
    // Return the first line found in the set with a matching tag
    for (uint32_t i = 0; i < config.assoc; i++)
        if (lines[start_idx + i].tag == tag)
            return &lines[start_idx + i];
    return nullptr;
}
