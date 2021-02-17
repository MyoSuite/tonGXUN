#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <eosio/transaction.hpp>

namespace eosiosystem {

   using eosio::current_time_point;
   using eosio::token;

   void system_contract::bidname( const name& bidder, const name& newname, const asset& bid ) {
      require_auth( bidder );
      check( newname.suffix() == newname, "you can only bid on top-level suffix" );

      check( (bool)newname, "the empty name is not a valid account name to bid on" );
      check( (newname.value & 0xFull) == 0, "13 character names are not valid account names to bid on" );
      check( (newname.value & 0x1F0ull) == 0, "accounts with 12 character names and no dots can be created without bidding required" );
      check( !is_account( newname ), "account already exists" );
      check( bid.symbol == core_symbol(), "asset must be system token" );
      check( bid.amount > 0, "insufficient bid" );
      token::transfer_action transfer_act{ token_account, { {bidder, active_permission} } };
      transfer_act.send( bidder, names_account, bid, std::string("bid name ")+ newname.to_string() );
      name_bid_table bids(get_self(), get_self().value);
      print( name{bidder}, " bid ", bid, " on ", name{newname}, "\n" );
      auto current = bids.find( newname.value );
      if( current == bids.end() ) {
         bids.emplace( bidder, [&]( auto& b ) {
            b.newname = newname;
            b.high_bidder = bidder;
            b.high_bid = bid.amount;
            b.l