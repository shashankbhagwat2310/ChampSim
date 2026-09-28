#include "lip.h"

#include <algorithm>
#include <cassert>

lip::lip(CACHE* cache)
    : lip(cache, cache->NUM_SET, cache->NUM_WAY)
{
}

lip::lip(CACHE* cache, long sets, long ways)
    : replacement(cache),
      NUM_WAY(ways),
      last_used_cycles(static_cast<std::size_t>(sets * ways), 0)
{
}

long lip::find_victim(uint32_t triggering_cpu,
                      uint64_t instr_id,
                      long set,
                      const champsim::cache_block* current_set,
                      champsim::address ip,
                      champsim::address full_addr,
                      access_type type)
{
  auto begin = std::next(std::begin(last_used_cycles), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);

  auto victim = std::min_element(begin, end);

  assert(begin <= victim);
  assert(victim < end);

  return std::distance(begin, victim);
}

void lip::replacement_cache_fill(uint32_t triggering_cpu,
                                 long set,
                                 long way,
                                 champsim::address full_addr,
                                 champsim::address ip,
                                 champsim::address victim_addr,
                                 access_type type)
{
  auto begin = std::next(std::begin(last_used_cycles), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);

  auto lru_position = std::min_element(begin, end);

  last_used_cycles.at(
      static_cast<std::size_t>(set * NUM_WAY + way)
  ) = *lru_position - 1;
}

void lip::update_replacement_state(uint32_t triggering_cpu,
                                   long set,
                                   long way,
                                   champsim::address full_addr,
                                   champsim::address ip,
                                   champsim::address victim_addr,
                                   access_type type,
                                   uint8_t hit)
{
  if (hit && access_type{type} != access_type::WRITE)
  {
    last_used_cycles.at(
        static_cast<std::size_t>(set * NUM_WAY + way)
    ) = cycle++;
  }
}