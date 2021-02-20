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
  _grotat