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
   static inline auto set_field( F flags, E field, bool value = true )
   -> std::enable_if_t< std::is_integral_v<F> && std::is_unsigned_v<F> &&
                        std::is_enum_v<E> && std::is_same_v< F, std::underlying_type_t<E> >, F >
   {
      if( value )
         return ( flags | static_cast<F>(field) );
      else
         return ( flags & ~static_cast<F>(field) );
   }

   static constexpr uint32_t seconds_per_year      = 52 * 7 * 24 * 3600;
   static constexpr uint32_t seconds_per_day       = 24 * 3600;
   static constexpr uint32_t seconds_per_hour      = 3600;
   static constexpr int64_t  useconds_per_year     = int64_t(seconds_per_year) * 1000'000ll;
   static constexpr int64_t  useconds_per_day      = int64_t(seconds_per_day) * 1000'000ll;
   static constexpr int64_t  useconds_per_hour     = int64_t(seconds_per_hour) * 1000'000ll;
   static constexpr uint32_t blocks_per_day        = 2 * seconds_per_day; // half seconds per day

   static constexpr int64_t  min_activated_stake   = 150'000'000'0000;
   static constexpr int64_t  ram_gift_bytes        = 1400;
   static constexpr int64_t  min_pervote_daily_pay = 100'0000;
   static constexpr uint32_t refund_delay_sec      = 3 * seconds_per_day;

   static constexpr int64_t  inflation_precision           = 100;     // 2 decimals
   static constexpr int64_t  default_annual_rate           = 500;     // 5% annual rate
   static constexpr int64_t  pay_factor_precision          = 10000;
   static constexpr int64_t  default_inflation_pay_factor  = 50000;   // producers pay share = 10000 / 50000 = 20% of the inflation
   static constexpr int64_t  default_votepay_factor        = 40000;   // per-block pay share = 10000 / 40000 = 25% of the producer pay

#ifdef SYSTEM_BLOCKCHAIN_PARAMETERS
   struct blockchain_parameters_v1 : eosio::blockchain_parameters
   {
      eosio::binary_extension<uint32_t> max_action_return_value_size;
      EOSLIB_SERIALIZE_DERIVED( blockchain_parameters_v1, eosio::blockchain_parameters,
                                (max_action_return_value_size) )
   };
   using blockchain_parameters_t = blockchain_parameters_v1;
#else
   using blockchain_parameters_t = eosio::blockchain_parameters;
#endif

   // BEGIN TELOS
   /*
    * NOTE: 1000 is used only to make the unit tests pass.
    * the number that is set on the main net is 1,000,000.
	* This value should be changed by ABPs before launching.
    */
   const uint32_t block_num_network_activation = 1000;

   const uint64_t max_bpay_rate = 6000;
   const uint64_t max_worker_monthly_amount = 1'000'000'0000;

   struct[[ eosio::table, eosio::contract("eosio.system") ]] payment_info {
     name bp;
     asset pay;

     uint64_t primary_key() const { return bp.value; }
   };

   typedef eosio::multi_index< "payments"_n, payment_info > payments_table;

   struct [[eosio::table("schedulemetr"), eosio::contract("eosio.system")]] schedule_metrics_state {
     name                             last_onblock_caller;
     int32_t                          block_counter_correction;
     std::vector<producer_metric>     producers_metric;

     uint64_t primary_key()const { return last_onblock_caller.value; }
   };

   typedef eosio::singleton< "schedulemetr"_n, schedule_metrics_state > schedule_metrics_singleton;

   struct [[eosio::table("rotations"), eosio::contract("eosio.system")]] rotation_state {
      // bool                            is_rotation_active = true;
      name                    bp_currently_out;
      name                    sbp_currently_in;
      uint32_t                bp_out_index;
      uint32_t                sbp_in_index;
      block_timestamp         next_rotation_time;
      block_timestamp         last_rotation_time;

      //NOTE: This might not be the best place for this information

      // bool                            is_kick_active = true;
      // account_name                    last_onblock_caller;
      // block_timestamp                 last_time_block_produced;
   };

   typedef eosio::singleton< "rotations"_n, rotation_state> rotation_singleton;


   struct[[ eosio::table("payrate"), eosio::contract("eosio.system") ]] payrates {
      uint64_t bpay_rate;
      uint64_t worker_amount;
      uint64_t primary_key() const { return bpay_rate; }
      EOSLIB_SERIALIZE(payrates, (bpay_rate)(worker_amount))
   };

   typedef eosio::singleton< "payrate"_n, payrates > payrate_singleton;


   enum class kick_type {
      REACHED_TRESHOLD = 1,
