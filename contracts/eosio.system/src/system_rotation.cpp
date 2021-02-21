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
        prods.empla