#include "dip.h"

#include <algorithm>
#include <cassert>

dip::dip(CACHE* cache)
    : dip(cache, cache->NUM_SET, cache->NUM_WAY)
{
}

dip::dip(CACHE* cache, long sets, long ways)
    : replacement(cache),
      NUM_WAY(ways),
      last_used_cycles(static_cast<std::size_t>(sets * ways), 0)
{
}

bool dip::is_lru_leader(long set) const
{
  return set >= 0 && set < NUM_LEADER_SETS;
}

bool dip::is_bip_leader(long set) const
{
  return set >= NUM_LEADER_SETS &&
         set < (2 * NUM_LEADER_SETS);
}

void dip::insert_lru(long set, long way)
{
  last_used_cycles.at(
      static_cast<std::size_t>(set * NUM_WAY + way)
  ) = cycle++;
}

void dip::insert_lip(long set, long way)
{
  auto begin =
      std::next(std::begin(last_used_cycles), set * NUM_WAY);

  auto end = std::next(begin, NUM_WAY);

  auto lru_position = std::min_element(begin, end);

  last_used_cycles.at(
      static_cast<std::size_t>(set * NUM_WAY + way)
  ) = *lru_position - 1;
}

void dip::insert_bip(long set, long way)
{
  if ((bip_counter++ % 32) == 0)
  {
    insert_lru(set, way);
  }
  else
  {
    insert_lip(set, way);
  }
}

long dip::find_victim(uint32_t triggering_cpu,
                      uint64_t instr_id,
                      long set,
                      const champsim::cache_block* current_set,
                      champsim::address ip,
                      champsim::address full_addr,
                      access_type type)
{
  auto begin =
      std::next(std::begin(last_used_cycles), set * NUM_WAY);

  auto end = std::next(begin, NUM_WAY);

  auto victim = std::min_element(begin, end);

  assert(begin <= victim);
  assert(victim < end);

  return std::distance(begin, victim);
}

void dip::replacement_cache_fill(uint32_t triggering_cpu,
                                 long set,
                                 long way,
                                 champsim::address full_addr,
                                 champsim::address ip,
                                 champsim::address victim_addr,
                                 access_type type)
{
  /*
   * Leader sets:
   *
   * 0 - 31  : LRU leader sets
   * 32 - 63 : BIP leader sets
   *
   * Remaining sets are follower sets.
   */

  if (is_lru_leader(set))
  {
    // LRU leader miss.
    if (psel < PSEL_MAX)
      ++psel;

    insert_lru(set, way);
    return;
  }

  if (is_bip_leader(set))
  {
    // BIP leader miss.
    if (psel > 0)
      --psel;

    insert_bip(set, way);
    return;
  }

  /*
   * Follower sets:
   *
   * PSEL >= 512 -> LRU
   * PSEL <  512 -> BIP
   */
  if (psel >= PSEL_THRESHOLD)
  {
    insert_lru(set, way);
  }
  else
  {
    insert_bip(set, way);
  }
}

void dip::update_replacement_state(uint32_t triggering_cpu,
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