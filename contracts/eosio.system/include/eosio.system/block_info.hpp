#pragma once

#include <eosio/multi_index.hpp>
#include <eosio/name.hpp>
#include <eosio/time.hpp>

#include <limits>
#include <optional>

namespace eosiosystem::block_info {

static constexpr uint32_t rolling_window_size = 10;

/**
 * The blockinfo table holds a rolling window of records containing information for recent blocks.
 *
 * Each record stores the height and timestamp of the correspond block.
 * A record is added for a new block through the onblock action.
 * The onblock action also erases up to two old records at a time in an attempt to keep the table consisting of only
 * records for blocks going back a particular block height difference backward from the most recent block.
 * Currently that block height difference is hardcoded to 10.
 */
struct [[eosio::table, eosio::contract("eosio.system")]] block_info_record
{
   uint8_t           version = 0;
   uint32_t          block_height;
   eosio::time_point block_timestamp;

   uint64_t primar