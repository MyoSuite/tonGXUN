#pragma once

#include <eosio/action.hpp>
#include <eosio/binary_extension.hpp>
#include <eosio/contract.hpp>
#include <eosio/crypto.hpp>
#include <eosio/fixed_bytes.hpp>
#include <eosio/ignore.hpp>
#include <eosio/print.hpp>
#include <eosio/privileged.hpp>
#include <eosio/producer_schedule.hpp>

namespace eosiosystem {

   using eosio::binary_extension;
   using eosio::checksum256;
   using eosio::ignore;
   using eosio::name;
   using eosio::permission_level;
   using eosio::public_key;

   // TELOS BEGIN
   struct producer_metric {
      name bp_name;
      uint32_t missed_blocks_per_cycle = 12;

      // explicit serialization macro is not necessary, used here only to improve
      // compilation time
      EOSLIB_SERIALIZE(producer_metric, (bp_name)(missed_blocks_per_cycle))
   };
