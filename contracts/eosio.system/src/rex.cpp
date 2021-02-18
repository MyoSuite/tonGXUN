
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/rex.results.hpp>

namespace eosiosystem {

   using eosio::current_time_point;
   using eosio::token;
   using eosio::seconds;

   void system_contract::deposit( const name& owner, const asset& amount )
   {
      require_auth( owner );

      check( amount.symbol == core_symbol(), "must deposit core token" );
      check( 0 < amount.amount, "must deposit a positive amount" );
      // inline transfer from owner's token balance
      {
         token::transfer_action transfer_act{ token_account, { owner, active_permission } };
         transfer_act.send( owner, rex_account, amount, "deposit to REX fund" );
      }
      transfer_to_fund( owner, amount );
   }

   void system_contract::withdraw( const name& owner, const asset& amount )
   {
      require_auth( owner );

      check( amount.symbol == core_symbol(), "must withdraw core token" );
      check( 0 < amount.amount, "must withdraw a positive amount" );
      update_rex_account( owner, asset( 0, core_symbol() ), asset( 0, core_symbol() ) );
      transfer_from_fund( owner, amount );
      // inline transfer to owner's token balance
      {
         token::transfer_action transfer_act{ token_account, { rex_account, active_permission } };
         transfer_act.send( rex_account, owner, amount, "withdraw from REX fund" );
      }
   }

   void system_contract::buyrex( const name& from, const asset& amount )
   {
      require_auth( from );

      check( amount.symbol == core_symbol(), "asset must be core token" );
      check( 0 < amount.amount, "must use positive amount" );
      /* TELOS Remove requirement to vote 21 BPs or select a proxy to stake to REX
      check_voting_requirement( from );
      */
      transfer_from_fund( from, amount );
      const asset rex_received    = add_to_rex_pool( amount );
      const asset delta_rex_stake = add_to_rex_balance( from, amount, rex_received );
      runrex(2);
      update_rex_account( from, asset( 0, core_symbol() ), delta_rex_stake );
      // dummy action added so that amount of REX tokens purchased shows up in action trace
      rex_results::buyresult_action buyrex_act( rex_account, std::vector<eosio::permission_level>{ } );
      buyrex_act.send( rex_received );
   }

   void system_contract::unstaketorex( const name& owner, const name& receiver, const asset& from_net, const asset& from_cpu )
   {
      require_auth( owner );

      check( from_net.symbol == core_symbol() && from_cpu.symbol == core_symbol(), "asset must be core token" );
      check( (0 <= from_net.amount) && (0 <= from_cpu.amount) && (0 < from_net.amount || 0 < from_cpu.amount),
             "must unstake a positive amount to buy rex" );
      /* TELOS Remove requirement to vote 21 BPs or select a proxy to stake to REX
      check_voting_requirement( from );
      */

      {
         del_bandwidth_table dbw_table( get_self(), owner.value );
         auto del_itr = dbw_table.require_find( receiver.value, "delegated bandwidth record does not exist" );
         check( from_net.amount <= del_itr->net_weight.amount, "amount exceeds tokens staked for net");
         check( from_cpu.amount <= del_itr->cpu_weight.amount, "amount exceeds tokens staked for cpu");
         dbw_table.modify( del_itr, same_payer, [&]( delegated_bandwidth& dbw ) {
            dbw.net_weight.amount -= from_net.amount;
            dbw.cpu_weight.amount -= from_cpu.amount;
         });
         if ( del_itr->is_empty() ) {
            dbw_table.erase( del_itr );
         }
      }

      update_resource_limits( name(0), receiver, -from_net.amount, -from_cpu.amount );

      const asset payment = from_net + from_cpu;
      // inline transfer from stake_account to rex_account
      {
         token::transfer_action transfer_act{ token_account, { stake_account, active_permission } };
         transfer_act.send( stake_account, rex_account, payment, "buy REX with staked tokens" );
      }
      const asset rex_received = add_to_rex_pool( payment );
      auto rex_stake_delta = add_to_rex_balance( owner, payment, rex_received );
      runrex(2);
      update_rex_account( owner, asset( 0, core_symbol() ), rex_stake_delta - payment, true );
      // dummy action added so that amount of REX tokens purchased shows up in action trace
      rex_results::buyresult_action buyrex_act( rex_account, std::vector<eosio::permission_level>{ } );
      buyrex_act.send( rex_received );
   }

   void system_contract::sellrex( const name& from, const asset& rex )
   {
      require_auth( from );

      runrex(2);

      auto bitr = _rexbalance.require_find( from.value, "user must first buyrex" );
      check( rex.amount > 0 && rex.symbol == bitr->rex_balance.symbol,
             "asset must be a positive amount of (REX, 4)" );
      process_rex_maturities( bitr );
      check( rex.amount <= bitr->matured_rex, "insufficient available rex" );

      const auto current_order = fill_rex_order( bitr, rex );