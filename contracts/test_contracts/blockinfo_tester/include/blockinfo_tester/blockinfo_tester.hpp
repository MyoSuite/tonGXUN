#pragma once

#ifdef TEST_INCLUDE

#include <fc/io/varint.hpp>
#include <fc/time.hpp>

#else

#include <eosio/time.hpp>
#include <eosio/varint.hpp>

#include <eosio.system/block_info.hpp>

#endif

#include <cstdint>
#include <optional>
#include <variant>

namespace system_contracts::testing::test_contracts::blockinfo_tester {

#ifdef TEST_INCLUDE

using time_point = fc::time_point;
using varint     = fc::unsigned_int;

#else

using time_point = eosio::time_point;
using varint     = eosio::unsigned_int;

#endif

/**
 * @brief Input data structure for `get_latest_block_batch_info` RPC
 *
 * @details Use this struct as the input for a call to the `get_latest_block_batch_info` RPC. That call will return the
 * result as the `latest_block_batch_info_result` struct.
 */
struct get_latest_block_batch_info
{
   uint32_t batch_start_height_offset;
   uint32_t batch_size;
};

#ifdef TEST_INCLUDE

struct block_batch_info
{
   uint32_t   batch_start_height;
   time_point batch_start_timestamp;
   uint32_t   batch_current_end_height;
   time_point batch_curren