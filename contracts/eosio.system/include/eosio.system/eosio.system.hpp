#pragma once

#include <eosio/asset.hpp>
#include <eosio/binary_extension.hpp>
#include <eosio/privileged.hpp>
#include <eosio/producer_schedule.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

#include <eosio.system/exchange_state.hpp>
#include <eosio.system/native.hpp>

#include <deque>
#include <optional>
#include <string>
#include <type_traits>

// TELOS BEGIN
#include <cmath>
// TELOS END
#ifdef CHANNEL_RAM_AND_NAMEBID_FEES_TO_REX
#undef CHANNEL_RAM_AND_NAMEBID_FEES_TO_REX
#endif
// CHANNEL_RAM_AND_NAMEBID_FEES_TO_REX macro determines whether ramfee and namebid proceeds are
// channeled to REX pool. In order to stop these proceeds from being channeled, the macro must
// be set to 0.
#define CHANNEL_RAM_AND_NAMEBID_FEES_TO_REX 1

namespace eosiosystem {

   using eosio::asset;
   using eosio::binary_extension;
   using eosio::block_timestamp;
   using eosio::check;
   using eosio::const_mem_fun;
   using eosio::datastream;
   using eosio::indexed_by;
   using eosio::name;
   using eosio::same_payer;
   using eosio::symbol;
   using eosio::symbol_code;
   using eosio::time_point;
   using eosio::time_point_sec;
   using eosio::unsigned_int;
   // TELOS
   using producer_location_pair = std::pair<eosio::producer_authority, uint16_t>;

   inline constexpr int64_t powerup_frac = 1'000'000'000'000'000ll;  // 1.0 = 10^15

   template<typename E, typename F>
   static inline auto has_field( F flags, E field )
   -> std::enable_if_t< std::is_integral_v<F> && std::is_unsigned_v<F> &&
                        std::is_enum_v<E> && std::is_same_v< F, std::underlying_type_t<E> >, bool>
   {
      return ( (flags & static_cast<F>(field)) != 0 );
   }

   template<typename E, typename F>
   static inline auto 