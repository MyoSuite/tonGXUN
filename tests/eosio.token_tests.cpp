#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include "eosio.system_tester.hpp"

#include "Runtime/Runtime.h"

#include <fc/variant_object.hpp>

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

class eosio_token_tester : public tester {
public:

   eosio_token_tester() {
      produce_blocks( 2 );

      create_accounts( { "alice"_n, "bob"_n, "carol"_n, "eosio.token"_n } );
      produce_blocks( 2 );

      set_code( "eosio.token"_n, contracts::token_wasm() );
      set_abi( "eosio.token"_n, contracts::token_abi().data() );

      produce_blocks();

      const auto& accnt = control->db().get<account_object,by_name>( "eosio.token"_n );
      abi_def abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));
   }

   action_result push_action( const account_name& signer, const action_name &name, const variant_object &data ) {
      string action_type_name = abi_ser.get_action_type(name);

      action act;
      act.account = "eosio.token"_n;
      act.name    = name;
      act.data    = abi_ser.variant_to_binary( action_type_name, data, abi_serializer::create_yield_function(abi_serializer_max_time) );

      return base_tester::push_action( std::move(act), signer.to_uint64_t() );
   }

   fc::variant get_stats( const string& symbolname )
   {
      auto symb = eosio::chain::symbol::from_string(symbolname);
      auto symbol_code = symb.to_symbol_code().value;
      vector<char> data = get_row_by_account( "eosio.token"_n, name(symbol_code), "stat"_n, account_name(symbol_code) );
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "currency_stats", data, abi_serializer::create_yield_function(abi_serializer_max_time) );
   }

   