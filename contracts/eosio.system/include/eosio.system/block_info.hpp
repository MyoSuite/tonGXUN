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

   uint64_t primary_key() const { return block_height; }

   EOSLIB_SERIALIZE(block_info_record, (version)(block_height)(block_timestamp))
};

using block_info_table = eosio::multi_index<"blockinfo"_n, block_info_record>;

struct block_batch_info
{
   uint32_t          batch_start_height;
   eosio::time_point batch_start_timestamp;
   uint32_t          batch_current_end_height;
   eosio::time_point batch_current_end_timestamp;
};

struct latest_block_batch_info_result
{
   enum error_code_enum : uint32_t
   {
      no_error,
      invalid_input,
      unsupported_version,
      insufficient_data
   };

   std::optional<block_batch_info> result;
   error_code_enum                 error_code = no_error;
};

/**
 * Get information on the latest block batch.
 *
 * A block batch is a contiguous range of blocks of a particular size.
 * A sequence of blocks can be partitioned into a sequence of block batches, where all except for perhaps the last batch
 * in the sequence have the same size. The last batch in the sequence can have a smaller size if the
 * blocks of the blockchain that would complete that batch have not yet been generated or recorded.
 *
 * This function enables the caller to specify a particular partitioning of the sequence of blocks into a sequence of
 * block batches of a particular non-zero size (`batch_size`) and then isolates the last block batch in that sequence
 * and returns the information about that latest block batch if possible. The partitioning will ensure that
 * `batch_start_height_offset` will be equal to the starting block height of exactly one of block batches in the
 * sequence.
 *
 * The information about the latest block batch is the same data captured in `block_batch_info`.
 * Particularly, it returns the height and timestamp of starting and ending blocks within that latest block batch.
 * Note that the range spanning from the start to end block of the latest block batch may be less than batch_size
 * because latest block batch may be incomplete.
 * Also, it is possible for the record capturing info for the starting block to not exist in the blockinfo table. This
 * can either be due to the records being erased as they fall out of the rolling window or, in rare cases, due to gaps
 * in block info recor