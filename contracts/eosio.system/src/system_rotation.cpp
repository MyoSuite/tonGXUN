#include <eosio.system/eosio.system.hpp>

#define TWELVE_HOURS_US 43200000000
#define SIX_HOURS_US 21600000000
#define ONE_HOUR_US 900000000       // debug version
#define SIX_MINUTES_US 360000000    // debug version
#define TWELVE_MINUTES_US 720000000 // debug version
#define MAX_PRODUCERS 42     // revised for TEDP 2 Phase 2, also set in producer_pay.cpp, change in both places
#define TOP_PRODUCERS 21
#define MAX_VOTE_PRODUCERS 30

namespace eosiosystem {
using namespace eosio;

void system_contract::set_bps_rotation(name bpOut, name sbpIn) {
  _grotation.bp_currently_out = bpOut;
  _grotation.sbp_currently_in = sbpIn;
}

void system_contract::update_rotation_time(block_timestamp block_time) {
  _grotation.last_rotation_time = block_time;
  _grotation.next_rotation_time = block_timestamp(
      block_time.to_time_point() + time_point(microseconds(TWELVE_HOURS_US)));
}

void system_contract::update_missed_blocks_per_rotation() {
  auto active_schedule_size =
      std::distance(_gschedule_metrics.producers_metric.begin(),
                    _gschedule_metrics.producers_metric.end());
  uint16_t max_kick_bps = uint16_t(active_schedule_size / 7);

  std::vector<producer_info> prods;

  for (auto &pm : _gschedule_metrics.producers_metric) {
    auto pitr = _producers.find(pm.bp_name.value);
    if (pitr != _producers.end() && pitr->is_active) {
      if (pm.missed_blocks_per_cycle > 0) {
        //  print("\nblock producer: ", name{pm.name}, " missed ",
        //  pm.missed_blocks_per_cycle, " blocks.");
        _producers.modify(pitr, same_payer, [&](auto &p) {
          p.missed_blocks_per_rotation += pm.missed_blocks_per_cycle;
          //   print("\ntotal missed blocks: ", p.missed_blocks_per_rotation);
        });
      }

      if (pitr->missed_blocks_per_rotation > 0)
        prods.emplace_back(*pitr);
    }
  }

  std::sort(prods.begin(), prods.end(), [](const producer_info &p1,
                                           const producer_info &p2) {
    if (p1.missed_blocks_per_rotation != p2.missed_blocks_per_rotation)
      return p1.missed_blocks_per_rotation > p2.missed_blocks_per_rotation;
    else
      return p1.total_votes < p2.total_votes;
  });

  for (auto &prod : prods) {
    auto pitr = _producers.find(prod.owner.value);

    if (crossed_missed_blocks_threshold(pitr->missed_blocks_per_rotation,
                                        uint32_t(active_schedule_size)) &&
        max_kick_bps > 0) {
      _producers.modify(pitr, same_payer, [&](auto &p) {
        p.lifetime_missed_blocks += p.missed_blocks_per_rotation;
        p.kick(kick_type::REACHED_TRESHOLD);
      });
      max_kick_bps--;
    } else
      break;
  }
}

void system_contract::restart_missed_blocks_per_rotation(
    std::vector<producer_location_pair> prods) {
  // restart all missed blocks to bps and sbps
  for (size_t i = 0; i < prods.size(); i++) {
    auto bp_name = prods[i].first.producer_name;
    auto pitr = _producers.find(bp_name.value);

    if (pitr != _producers.end()) {
      _producers.modify(pitr, same_payer, [&](auto &p) {
        if (p.times_kicked > 0 && p.missed_blocks_per_rotation == 0) {
          p.times_kicked--;
        }
        p.lifetime_missed_blocks += p.missed_blocks_per_rotation;
        p.missed_blocks_per_rotation = 0;
      });
    }
  }
}

 bool system_contract::is_in_range(int32_t index, int32_t low_bound, int32_t up_bound) {
     return index >= low_bound && index < up_bound;
   } 

std::vector<producer_location_pair> system_contract::check_rotation_state( std::vector<producer_location_pair> prods, block_timestamp block_time) {
      uint32_t total_active_voted_prods = prods.size(); 
      std::vector<producer_location_pair>::iterator it_bp = prods.end();
      std::vector<producer_location_pair>::iterator it_sbp = prods.end();

      if (_grotation.next_rotation_time <= block_time) {

        if (total_active_voted_prods > TOP_PRODUCERS) {
          _grotation.bp_out_index = _grotation.bp_out_index >= TOP_PRODUCERS - 1 ? 0