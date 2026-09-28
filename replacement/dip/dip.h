#ifndef REPLACEMENT_DIP_H
#define REPLACEMENT_DIP_H

#include <cstdint>
#include <vector>

#include "cache.h"
#include "modules.h"

class dip : public champsim::modules::replacement
{
  long NUM_WAY;

  std::vector<int64_t> last_used_cycles;

  uint64_t cycle = 0;

  // BIP insertion counter.
  uint64_t bip_counter = 0;

  // 10-bit PSEL.
  uint16_t psel = 512;

  static constexpr uint16_t PSEL_MAX = 1023;
  static constexpr uint16_t PSEL_THRESHOLD = 512;

  static constexpr long NUM_LEADER_SETS = 32;

  bool is_lru_leader(long set) const;
  bool is_bip_leader(long set) const;

  void insert_lru(long set, long way);
  void insert_lip(long set, long way);
  void insert_bip(long set, long way);

public:
  explicit dip(CACHE* cache);
  dip(CACHE* cache, long sets, long ways);

  long find_victim(uint32_t triggering_cpu,
                   uint64_t instr_id,
                   long set,
                   const champsim::cache_block* current_set,
                   champsim::address ip,
                   champsim::address full_addr,
                   access_type type);

  void replacement_cache_fill(uint32_t triggering_cpu,
                              long set,
                              long way,
                              champsim::address full_addr,
                              champsim::address ip,
                              champsim::address victim_addr,
                              access_type type);

  void update_replacement_state(uint32_t triggering_cpu,
                                long set,
                                long way,
                                champsim::address full_addr,
                                champsim::address ip,
                                champsim::address victim_addr,
                                access_type type,
                                uint8_t hit);
};

#endif