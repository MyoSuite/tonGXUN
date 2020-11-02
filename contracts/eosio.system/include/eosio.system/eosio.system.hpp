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
      //  PREVENT_LIB_STOP_MOVING = 2,
      BPS_VOTING = 2
   };
   // END TELOS

  /**
   * The `eosio.system` smart contract is provided by `block.one` as a sample system contract, and it defines the structures and actions needed for blockchain's core functionality.
   * 
   * Just like in the `eosio.bios` sample contract implementation, there are a few actions which are not implemented at the contract level (`newaccount`, `updateauth`, `deleteauth`, `linkauth`, `unlinkauth`, `canceldelay`, `onerror`, `setabi`, `setcode`), they are just declared in the contract so they will show in the contract's ABI and users will be able to push those actions to the chain via the account holding the `eosio.system` contract, but the implementation is at the EOSIO core level. They are referred to as EOSIO native actions.
   * 
   * - Users can stake tokens for CPU and Network bandwidth, and then vote for producers or
   *    delegate their vote to a proxy.
   * - Producers register in order to be voted for, and can claim per-block and per-vote rewards.
   * - Users can buy and sell RAM at a market-determined price.
   * - Users can bid on premium names.
   * - A resource exchange system (REX) allows token holders to lend their tokens,
   *    and users to rent CPU and Network resources in return for a market-determined fee.
   */

   // A name bid, which consists of:
   // - a `newname` name that the bid is for
   // - a `high_bidder` account name that is the one with the highest bid so far
   // - the `high_bid` which is amount of highest bid
   // - and `last_bid_time` which is the time of the highest bid
   struct [[eosio::table, eosio::contract("eosio.system")]] name_bid {
     name            newname;
     name            high_bidder;
     int64_t         high_bid = 0; ///< negative high_bid == closed auction waiting to be claimed
     time_point      last_bid_time;

     uint64_t primary_key()const { return newname.value;                    }
     uint64_t by_high_bid()const { return static_cast<uint64_t>(-high_bid); }
   };

   // A bid refund, which is defined by:
   // - the `bidder` account name owning the refund
   // - the `amount` to be refunded
   struct [[eosio::table, eosio::contract("eosio.system")]] bid_refund {
      name         bidder;
      asset        amount;

      uint64_t primary_key()const { return bidder.value; }
   };
   typedef eosio::multi_index< "namebids"_n, name_bid,
                               indexed_by<"highbid"_n, const_mem_fun<name_bid, uint64_t, &name_bid::by_high_bid>  >
                             > name_bid_table;

   typedef eosio::multi_index< "bidrefunds"_n, bid_refund > bid_refund_table;

   // Defines new global state parameters.
   struct [[eosio::table("global"), eosio::contract("eosio.system")]] eosio_global_state : eosio::blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_reserved; }

      uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;  // TELOS SPECIFIC
      uint64_t             total_ram_bytes_reserved = 0;
      int64_t              total_ram_stake = 0;

      block_timestamp      last_producer_schedule_update;
      block_timestamp      last_proposed_schedule_update;  // TELOS SPECIFIC
      time_point           last_pervote_bucket_fill;
      int64_t              pervote_bucket = 0;
      int64_t              perblock_bucket = 0;
      uint32_t             total_unpaid_blocks = 0; /// all blocks which have been produced but not paid
      int64_t              total_activated_stake = 0;
      time_point           thresh_activated_stake_time;
      uint16_t             last_producer_schedule_size = 0;
      double               total_producer_vote_weight = 0; /// the sum of all producer votes
      block_timestamp      last_name_close;
      // BEGIN TELOS
      uint32_t             block_num = 12;
      uint32_t             last_claimrewards = 0;
      uint32_t             next_payment = 0;
      uint16_t             new_ram_per_block = 0;
      block_timestamp      last_ram_increase;
      block_timestamp      last_block_num; /* deprecated */
      double               total_producer_votepay_share = 0;
      uint8_t              revision = 0; ///< used to track version updates in the future.
      // END TELOS

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE_DERIVED( eosio_global_state, eosio::blockchain_parameters,
                                (max_ram_size)(total_ram_bytes_reserved)(total_ram_stake)
                                (last_producer_schedule_update)(last_proposed_schedule_update)(last_pervote_bucket_fill)
                                (pervote_bucket)(perblock_bucket)(total_unpaid_blocks)(total_activated_stake)(thresh_activated_stake_time)
                                (last_producer_schedule_size)(total_producer_vote_weight)(last_name_close)(block_num)(last_claimrewards)(next_payment)
                                (new_ram_per_block)(last_ram_increase)(last_block_num)(total_producer_votepay_share)(revision) )
   };

   // Defines new global state parameters added after version 1.0
   struct [[eosio::table("global2"), eosio::contract("eosio.system")]] eosio_global_state2 {
      eosio_global_state2(){}

      uint16_t          new_ram_per_block = 0;
      block_timestamp   last_ram_increase;
      block_timestamp   last_block_num; /* deprecated */
      double            total_producer_votepay_share = 0;
      uint8_t           revision = 0; ///< used to track version updates in the future.

      EOSLIB_SERIALIZE( eosio_global_state2, (new_ram_per_block)(last_ram_increase)(last_block_num)
                        (total_producer_votepay_share)(revision) )
   };

   // Defines new global state parameters added after version 1.3.0
   struct [[eosio::table("global3"), eosio::contract("eosio.system")]] eosio_global_state3 {
      eosio_global_state3() { }
      time_point        last_vpay_state_update;
      double            total_vpay_share_change_rate = 0;

      EOSLIB_SERIALIZE( eosio_global_state3, (last_vpay_state_update)(total_vpay_share_change_rate) )
   };

   // Defines new global state parameters to store inflation rate and distribution
   struct [[eosio::table("global4"), eosio::contract("eosio.system")]] eosio_global_state4 {
      eosio_global_state4() { }
      double   continuous_rate;
      int64_t  inflation_pay_factor;
      int64_t  votepay_factor;

      EOSLIB_SERIALIZE( eosio_global_state4, (continuous_rate)(inflation_pay_factor)(votepay_factor) )
   };

   inline eosio::block_signing_authority convert_to_block_signing_authority( const eosio::public_key& producer_key ) {
      return eosio::block_signing_authority_v0{ .threshold = 1, .keys = {{producer_key, 1}} };
   }

   // Defines `producer_info` structure to be stored in `producer_info` table, added after version 1.0
   struct [[eosio::table, eosio::contract("eosio.system")]] producer_info {
      name                                                     owner;
      double                                                   total_votes = 0;
      eosio::public_key                                        producer_key; /// a packed public key object
      bool                                                     is_active = true;
      // TELOS BEGIN
      std::string                                              unreg_reason;
      // TELOS END
      std::string                                              url;
      uint32_t                                                 unpaid_blocks = 0;
      // TELOS BEGIN
      uint32_t                                                 lifetime_produced_blocks = 0;
      uint32_t                                                 missed_blocks_per_rotation = 0;
      uint32_t                                                 lifetime_missed_blocks;
      // TELOS END
      time_point                                               last_claim_time;
      uint16_t                                                 location = 0;
      // TELOS BEGIN
      uint32_t                                                 kick_reason_id = 0;
      std::string                                              kick_reason;
      uint32_t                                                 times_kicked = 0;
      uint32_t                                                 kick_penalty_hours = 0;
      block_timestamp                                          last_time_kicked;
      // TELOS END
      eosio::binary_extension<eosio::block_signing_authority>  producer_authority; // added in version 1.9.0

      uint64_t primary_key()const { return owner.value;                             }
      double   by_votes()const    { return is_active ? -total_votes : total_votes;  }
      bool     active()const      { return is_active;                               }
      void     deactivate()       { producer_key = public_key(); producer_authority.reset(); is_active = false; }

      // TELOS BEGIN
      void kick(kick_type kt, uint32_t penalty = 0) {
         times_kicked++;
         last_time_kicked = block_timestamp(eosio::current_time_point());

         if(penalty == 0) kick_penalty_hours  = uint32_t(std::pow(2, times_kicked));

         switch(kt) {
           case kick_type::REACHED_TRESHOLD:
             kick_reason_id = uint32_t(kick_type::REACHED_TRESHOLD);
             kick_reason = "Producer account was deactivated because it reached the maximum missed blocks in this rotation timeframe.";
           break;
           case kick_type::BPS_VOTING:
             kick_reason_id = uint32_t(kick_type::BPS_VOTING);
             kick_reason = "Producer account was deactivated by vote.";
             kick_penalty_hours = penalty;
           break;
         }
         lifetime_missed_blocks += missed_blocks_per_rotation;
         missed_blocks_per_rotation = 0;
         // print("\nblock producer: ", name{owner}, " was kicked.");
         deactivate();
      }
      // TELOS END

      eosio::block_signing_authority get_producer_authority()const {
         if( producer_authority.has_value() ) {
            bool zero_threshold = std::visit( [](auto&& auth ) -> bool {
               return (auth.threshold == 0);
            }, *producer_authority );
            // zero_threshold could be true despite the validation done in regproducer2 because the v1.9.0 eosio.system
            // contract has a bug which may have modified the producer table such that the producer_authority field
            // contains a default constructed eosio::block_signing_authority (which has a 0 threshold and so is invalid).
            if( !zero_threshold ) return *producer_authority;
         }
         return convert_to_block_signing_authority( producer_key );
      }

      // The unregprod and claimrewards actions modify unrelated fields of the producers table and under the default
      // serialization behavior they would increase the size of the serialized table if the producer_authority field
      // was not already present. This is acceptable (though not necessarily desired) because those two actions require
      // the aut