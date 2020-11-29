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
      // the authority of the producer who pays for the table rows.
      // However, the rmvproducer action and the onblock transaction would also modify the producer table in a similar
      // way and increasing its serialized size is not acceptable in that context.
      // So, a custom serialization is defined to handle the binary_extension producer_authority
      // field in the desired way. (Note: v1.9.0 did not have this custom serialization behavior.)

      // TELOS EDITED WITH CUSTOM FIELDS
      template<typename DataStream>
      friend DataStream& operator << ( DataStream& ds, const producer_info& t ) {
         ds << t.owner
            << t.total_votes
            << t.producer_key
            << t.is_active
            << t.unreg_reason
            << t.url
            << t.unpaid_blocks
            << t.lifetime_produced_blocks
            << t.missed_blocks_per_rotation
            << t.lifetime_missed_blocks
            << t.last_claim_time
            << t.location
            << t.kick_reason_id
            << t.kick_reason
            << t.times_kicked
            << t.kick_penalty_hours
            << t.last_time_kicked;

         if( !t.producer_authority.has_value() ) return ds;

         return ds << t.producer_authority;
      }

      // TELOS EDITED WITH CUSTOM FIELDS
      template<typename DataStream>
      friend DataStream& operator >> ( DataStream& ds, producer_info& t ) {
         return ds >> t.owner
                   >> t.total_votes
                   >> t.producer_key
                   >> t.is_active
                   >> t.unreg_reason
                   >> t.url
                   >> t.unpaid_blocks
                   >> t.lifetime_produced_blocks
                   >> t.missed_blocks_per_rotation
                   >> t.lifetime_missed_blocks
                   >> t.last_claim_time
                   >> t.location
                   >> t.kick_reason_id
                   >> t.kick_reason
                   >> t.times_kicked
                   >> t.kick_penalty_hours
                   >> t.last_time_kicked
                   >> t.producer_authority;
      }
   };

   // Defines new producer info structure to be stored in new producer info table, added after version 1.3.0
   struct [[eosio::table, eosio::contract("eosio.system")]] producer_info2 {
      name            owner;
      double          votepay_share = 0;
      time_point      last_votepay_share_update;

      uint64_t primary_key()const { return owner.value; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( producer_info2, (owner)(votepay_share)(last_votepay_share_update) )
   };

   // Voter info. Voter info stores information about the voter:
   // - `owner` the voter
   // - `proxy` the proxy set by the voter, if any
   // - `producers` the producers approved by this voter if no proxy set
   // - `staked` the amount staked
   struct [[eosio::table, eosio::contract("eosio.system")]] voter_info {
      name                owner;     /// the voter
      name                proxy;     /// the proxy set by the voter, if any
      std::vector<name>   producers; /// the producers approved by this voter if no proxy set
      int64_t             staked = 0;

      // TELOS BEGIN
      int64_t             last_stake = 0;
      // TELOS END

      //  Every time a vote is cast we must first "undo" the last vote weight, before casting the
      //  new vote weight.  Vote weight is calculated as:
      //  stated.amount * 2 ^ ( weeks_since_launch/weeks_per_year)
      double              last_vote_weight = 0; /// the vote weight cast the last time the vote was updated

      // Total vote weight delegated to this voter.
      double              proxied_vote_weight= 0; /// the total vote weight delegated to this voter as a proxy
      bool                is_proxy = 0; /// whether the voter is a proxy for others


      uint32_t            flags1 = 0;
      uint32_t            reserved2 = 0;
      eosio::asset        reserved3;

      uint64_t primary_key()const { return owner.value; }

      enum class flags1_fields : uint32_t {
         ram_managed = 1,
         net_managed = 2,
         cpu_managed = 4
      };

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( voter_info, (owner)(proxy)(producers)(staked)(last_stake)(last_vote_weight)(proxied_vote_weight)(is_proxy)(flags1)(reserved2)(reserved3) )
   };


   typedef eosio::multi_index< "voters"_n, voter_info >  voters_table;


   typedef eosio::multi_index< "producers"_n, producer_info,
                               indexed_by<"prototalvote"_n, const_mem_fun<producer_info, double, &producer_info::by_votes>  >
                             > producers_table;

   typedef eosio::multi_index< "producers2"_n, producer_info2 > producers_table2;


   typedef eosio::singleton< "global"_n, eosio_global_state >   global_state_singleton;

   typedef eosio::singleton< "global2"_n, eosio_global_state2 > global_state2_singleton;

   typedef eosio::singleton< "global3"_n, eosio_global_state3 > global_state3_singleton;

   typedef eosio::singleton< "global4"_n, eosio_global_state4 > global_state4_singleton;

   struct [[eosio::table, eosio::contract("eosio.system")]] user_resources {
      name          owner;
      asset         net_weight;
      asset         cpu_weight;
      int64_t       ram_bytes = 0;

      bool is_empty()const { return net_weight.amount == 0 && cpu_weight.amount == 0 && ram_bytes == 0; }
      uint64_t primary_key()const { return owner.value; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( user_resources, (owner)(net_weight)(cpu_weight)(ram_bytes) )
   };

   // Every user 'from' has a scope/table that uses every recipient 'to' as the primary key.
   struct [[eosio::table, eosio::contract("eosio.system")]] delegated_bandwidth {
      name          from;
      name          to;
      asset         net_weight;
      asset         cpu_weight;

      bool is_empty()const { return net_weight.amount == 0 && cpu_weight.amount == 0; }
      uint64_t  primary_key()const { return to.value; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( delegated_bandwidth, (from)(to)(net_weight)(cpu_weight) )

   };

   struct [[eosio::table, eosio::contract("eosio.system")]] refund_request {
      name            owner;
      time_point_sec  request_time;
      eosio::asset    net_amount;
      eosio::asset    cpu_amount;

      bool is_empty()const { return net_amount.amount == 0 && cpu_amount.amount == 0; }
      uint64_t  primary_key()const { return owner.value; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( refund_request, (owner)(request_time)(net_amount)(cpu_amount) )
   };


   typedef eosio::multi_index< "userres"_n, user_resources >      user_resources_table;
   typedef eosio::multi_index< "delband"_n, delegated_bandwidth > del_bandwidth_table;
   typedef eosio::multi_index< "refunds"_n, refund_request >      refunds_table;

   // `rex_pool` structure underlying the rex pool table. A rex pool table entry is defined by:
   // - `version` defaulted to zero,
   // - `total_lent` total amount of CORE_SYMBOL in open rex_loans
   // - `total_unlent` total amount of CORE_SYMBOL available to be lent (connector),
   // - `total_rent` fees received in exchange for lent  (connector),
   // - `total_lendable` total amount of CORE_SYMBOL that have been lent (total_unlent + total_lent),
   // - `total_rex` total number of REX shares allocated to contributors to total_lendable,
   // - `namebid_proceeds` the amount of CORE_SYMBOL to be transferred from namebids to REX pool,
   // - `loan_num` increments with each new loan
   struct [[eosio::table,eosio::contract("eosio.system")]] rex_pool {
      uint8_t    version = 0;
      asset      total_lent;
      asset      total_unlent;
      asset      total_rent;
      asset      total_lendable;
      asset      total_rex;
      asset      namebid_proceeds;
      uint64_t   loan_num = 0;

      uint64_t primary_key()const { return 0; }
   };

   typedef eosio::multi_index< "rexpool"_n, rex_pool > rex_pool_table;

   // `rex_return_pool` structure underlying the rex return pool table. A rex return pool table entry is defined by:
   // - `version` defaulted to zero,
   // - `last_dist_time` the last time proceeds from renting, ram fees, and name bids were added to the rex pool,
   // - `pending_bucket_time` timestamp of the pending 12-hour return bucket,
   // - `oldest_bucket_time` cached timestamp of the oldest 12-hour return bucket,
   // - `pending_bucket_proceeds` proceeds in the pending 12-hour return bucket,
   // - `current_rate_of_increase` the current rate per dist_interval at which proceeds are added to the rex pool,
   // - `proceeds` the maximum amount of proceeds that can be added to the rex pool at any given time
   struct [[eosio::table,eosio::contract("eosio.system")]] rex_return_pool {
      uint8_t        version = 0;
      time_point_sec last_dist_time;
      time_point_sec pending_bucket_time      = time_point_sec::maximum();
      time_point_sec oldest_bucket_time       = time_point_sec::min();
      int64_t        pending_bucket_proceeds  = 0;
      int64_t        current_rate_of_increase = 0;
      int64_t        proceeds                 = 0;

      static constexpr uint32_t total_intervals  = 30 * 144; // 30 days
      static constexpr uint32_t dist_interval    = 10 * 60;  // 10 minutes
      static constexpr uint8_t  hours_per_bucket = 12;
      static_assert( total_intervals * dist_interval == 30 * seconds_per_day );

      uint64_t primary_key()const { return 0; }
   };

   typedef eosio::multi_index< "rexretpool"_n, rex_return_pool > rex_return_pool_table;

   struct pair_time_point_sec_int64 {
      time_point_sec first;
      int64_t        second;

      EOSLIB_SERIALIZE(pair_time_point_sec_int64, (first)(second));
   };

   // `rex_return_buckets` structure underlying the rex return buckets table. A rex return buckets table is defined by:
   // - `version` defaulted to zero,
   // - `return_buckets` buckets of proceeds accumulated in 12-hour intervals
   struct [[eosio::table,eosio::contract("eosio.system")]] rex_return_buckets {
      uint8_t                                version = 0;
      std::vector<pair_time_point_sec_int64> return_buckets;  // sorted by first field

      uint64_t primary_key()const { return 0; }
   };

   typedef eosio::multi_index< "retbuckets"_n, rex_return_buckets > rex_return_buckets_table;

   // `rex_fund` structure underlying the rex fund table. A rex fund table entry is defined by:
   // - `version` defaulted to zero,
   // - `owner` the owner of the rex fund,
   // - `balance` the balance of the fund.
   struct [[eosio::table,eosio::contract("eosio.system")]] rex_fund {
      uint8_t version = 0;
      name    owner;
      asset   balance;

      uint64_t primary_key()const { return owner.value; }
   };

   typedef eosio::multi_index< "rexfund"_n, rex_fund > rex_fund_table;

   // `rex_balance` structure underlying the rex balance table. A rex balance table entry is defined by:
   // - `version` defaulted to zero,
   // - `owner` the owner of the rex fund,
   // - `vote_stake` the amount of CORE_SYMBOL currently included in owner's vote,
   // - `rex_balance` the amount of REX owned by owner,
   // - `matured_rex` matured REX available for selling
   struct [[eosio::table,eosio::contract("eosio.system")]] rex_balance {
      uint8_t version = 0;
      name    owner;
      asset   vote_stake;
      asset   rex_balance;
      int64_t matured_rex = 0;
      std::vector<pair_time_point_sec_int64> rex_maturities; /// REX daily maturity buckets

      uint64_t primary_key()const { return owner.value; }
   };

   typedef eosio::multi_index< "rexbal"_n, rex_balance > rex_balance_table;

   // `rex_loan` structure underlying the `rex_cpu_loan_table` and `rex_net_loan_table`. A rex net/cpu loan table entry is defined by:
   // - `version` defaulted to zero,
   // - `from` account creating and paying for loan,
   // - `receiver` account receiving rented resources,
   // - `payment` SYS tokens paid for the loan,
   // - `balance` is the amount of SYS tokens available to be used for loan auto-renewal,
   // - `total_staked` total amount staked,
   // - `loan_num` loan number/id,
   // - `expiration` the expiration time when loan will be either closed or renewed
   //       If payment <= balance, the loan is renewed, and closed otherwise.
   struct [[eosio::table,eosio::contract("eosio.system")]] rex_loan {
      uint8_t             version = 0;
      name                from;
      name                receiver;
      asset               payment;
      asset               balance;
      asset               total_staked;
      uint64_t            loan_num;
      eosio::time_point   expiration;

      uint64_t primary_key()const { return loan_num;                   }
      uint64_t by_expr()const     { return expiration.elapsed.count(); }
      uint64_t by_owner()const    { return from.value;                 }
   };

   typedef eosio::multi_index< "cpuloan"_n, rex_loan,
                               indexed_by<"byexpr"_n,  const_mem_fun<rex_loan, uint64_t, &rex_loan::by_expr>>,
                               indexed_by<"byowner"_n, const_mem_fun<rex_loan, uint64_t, &rex_loan::by_owner>>
                             > rex_cpu_loan_table;

   typedef eosio::multi_index< "netloan"_n, rex_loan,
                               indexed_by<"byexpr"_n,  const_mem_fun<rex_loan, uint64_t, &rex_loan::by_expr>>,
                               indexed_by<"byowner"_n, const_mem_fun<rex_loan, uint64_t, &rex_loan::by_owner>>
                             > rex_net_loan_table;

   struct [[eosio::table,eosio::contract("eosio.system")]] rex_order {
      uint8_t             version = 0;
      name                owner;
      asset               rex_requested;
      asset               proceeds;
      asset               stake_change;
      eosio::time_point   order_time;
      bool                is_open = true;

      void close()                { is_open = false;    }
      uint64_t primary_key()const { return owner.value; }
      uint64_t by_time()const     { return is_open ? order_time.elapsed.count() : std::numeric_limits<uint64_t>::max(); }
   };

   typedef eosio::multi_index< "rexqueue"_n, rex_order,
                               indexed_by<"bytime"_n, const_mem_fun<rex_order, uint64_t, &rex_order::by_time>>> rex_order_table;

   struct rex_order_outcome {
      bool success;
      asset proceeds;
      asset stake_change;
   };

   struct powerup_config_resource {
      std::optional<int64_t>        current_weight_ratio;   // Immediately set weight_ratio to this amount. 1x = 10^15. 0.01x = 10^13.
                                                            //    Do not specify to preserve the existing setting or use the default;
                                                            //    this avoids sudden price jumps. For new chains which don't need
                                                            //    to gradually phase out staking and REX, 0.01x (10^13) is a good
                                                            //    value for both current_weight_ratio and target_weight_ratio.
      std::optional<int64_t>        target_weight_ratio;    // Linearly shrink weight_ratio to this amount. 1x = 10^15. 0.01x = 10^13.
                                                            //    Do not specify to preserve the existing setting or use the default.
      std::optional<int64_t>        assumed_stake_weight;   // Assumed stake weight for ratio calculations. Use the sum of total
                                                            //    staked and total rented by REX at the time the power market
                                                            //    is first activated. Do not specify to preserve the existing
                                                            //    setting (no default exists); this avoids sudden price jumps.
                                                            //    For new chains which don't need to phase out staking and REX,
                                                            //    10^12 is probably a good value.
      std::optional<time_point_sec> target_timestamp;       // Stop automatic weight_ratio shrinkage at this time. Once this
                                                            //    time hits, weight_ratio will be target_weight_ratio. Ignored
                                                            //    if current_weight_ratio == target_weight_ratio. Do not specify
                                                            //    this to preserve the existing setting (no default exists).
      std::optional<double>         exponent;               // Exponent of resource price curve. Must be >= 1. Do not specify
                                                            //    to preserve the existing setting or use the default.
      std::optional<uint32_t>       decay_secs;             // Number of seconds for the gap between adjusted resource
                                                            //    utilization and instantaneous resource utilization to shrink
                                                            //    by 63%. Do not specify to preserve the existing setting or
                                                            //    use the default.
      std::optional<asset>          min_price;              // Fee needed to reserve the entire resource market weight at the
                                                            //    minimum price. For example, this could be set to 0.005% of
                                                            //    total token supply. Do not specify to preserve the existing
                                                            //    setting or use the default.
      std::optional<asset>          max_price;              // Fee needed to reserve the entire resource market weight at the
                                                            //    maximum price. For example, this could be set to 10% of total
                                                            //    token supply. Do not specify to preserve the existing
                                                            //    setting (no default exists).

      EOSLIB_SERIALIZE( powerup_config_resource, (current_weight_ratio)(target_weight_ratio)(assumed_stake_weight)
                                                (target_timestamp)(exponent)(decay_secs)(min_price)(max_price)    )
   };

   struct powerup_config {
      powerup_config_resource  net;             // NET market configuration
      powerup_config_resource  cpu;             // CPU market configuration
      std::optional<uint32_t> powerup_days;     // `powerup` `days` argument must match this. Do not specify to preserve the
                                                //    existing setting or use the default.
      std::optional<asset>    min_powerup_fee;  // Fees below this amount are rejected. Do not specify to preserve the
                                                //    existing setting (no default exists).

      EOSLIB_SERIALIZE( powerup_config, (net)(cpu)(powerup_days)(min_powerup_fee) )
   };

   struct powerup_state_resource {
      static constexpr double   default_exponent   = 2.0;                  // Exponent of 2.0 means that the price to reserve a
                                                                           //    tiny amount of resources increases linearly
                                                                           //    with utilization.
      static constexpr uint32_t default_decay_secs = 1 * seconds_per_day;  // 1 day; if 100% of bandwidth resources are in a
                                                                           //    single loan, then, assuming no further powerup usage,
                                                                           //    1 day after it expires the adjusted utilization
                                                                           //    will be at approximately 37% and after 3 days
                                                                           //    the adjusted utilization will be less than 5%.

      uint8_t        version                 = 0;
      int64_t        weight                  = 0;                  // resource market weight. calculated; varies over time.
                                                                   //    1 represents the same amount of resources as 1
                                                                   //    satoshi of SYS staked.
      int64_t        weight_ratio            = 0;                  // resource market weight ratio:
                                                                   //    assumed_stake_weight / (assumed_stake_weight + weight).
                                                                   //    calculated; varies over time. 1x = 10^15. 0.01x = 10^13.
      int64_t        assumed_stake_weight    = 0;                  // Assumed stake weight for ratio calculations.
      int64_t        initial_weight_ratio    = powerup_frac;        // Initial weight_ratio used for linear shrinkage.
      int64_t        target_weight_ratio     = powerup_frac / 100;  // Linearly shrink the weight_ratio to this amount.
      time_point_sec initial_timestamp       = {};                 // When weight_ratio shrinkage started
      time_point_sec target_timestamp        = {};                 // Stop automatic weight_ratio shrinkage at this time. Once this
                                                                   //    time hits, weight_ratio will be target_weight_ratio.
      double         exponent                = default_exponent;   // Exponent of resource price curve.
      uint32_t       decay_secs              = default_decay_secs; // Number of seconds for the gap between adjusted resource
                                                                   //    utilization and instantaneous utilization to shrink by 63%.
      asset          min_price               = {};                 // Fee needed to reserve the entire resource market weight at
                                                                   //    the minimum price (defaults to 0).
      asset          max_price               = {};                 // Fee needed to reserve the entire resource market weight at
                                                                   //    the maximum price.
      int64_t        utilization             = 0;                  // Instantaneous resource utilization. This is the current
                                                                   //    amount sold. utilization <= weight.
      int64_t        adjusted_utilization    = 0;                  // Adjusted resource utilization. This is >= utilization and
                                                                   //    <= weight. It grows instantly but decays exponentially.
      time_point_sec utilization_timestamp   = {};                 // When adjusted_utilization was last updated
   };

   struct [[eosio::table("powup.state"),eosio::contract("eosio.system")]] powerup_state {
      static constexpr uint32_t default_powerup_days = 30; // 30 day resource powerup

      uint8_t                    version           = 0;
      powerup_state_resource     net               = {};                     // NET market state
      powerup_state_resource     cpu               = {};                     // CPU market state
      uint32_t                   powerup_days      = default_powerup_days;   // `powerup` `days` argument must match this.
      asset                      min_powerup_fee   = {};                     // fees below this amount are rejected

      uint64_t primary_key()const { return 0; }
   };

   typedef eosio::singleton<"powup.state"_n, powerup_state> powerup_state_singleton;

   struct [[eosio::table("powup.order"),eosio::contract("eosio.system")]] powerup_order {
      uint8_t              version = 0;
      uint64_t             id;
      name                 owner;
      int64_t              net_weight;
      int64_t              cpu_weight;
      time_point_sec       expires;

      uint64_t primary_key()const { return id; }
      uint64_t by_owner()const    { return owner.value; }
      