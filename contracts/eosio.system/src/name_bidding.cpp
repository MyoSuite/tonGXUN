#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <eosio/transaction.hpp>

namespace eosiosystem {

   using eosio::current_time_point;
   using eosio::token;

   void system_contract::bidname( const name& bidder, const name& newname, const asset& bid ) {
      require_auth( bidder );
      check( newname.suffix() == newname, "you can only bid o