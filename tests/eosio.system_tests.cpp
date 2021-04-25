#include <boost/test/unit_test.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fc/log/logger.hpp>
#include <eosio/chain/exceptions.hpp>
#include <Runtime/Runtime.h>

#include "eosio.system_tester.hpp"
struct _abi_hash {
   name owner;
   fc::sha256 hash;
};
FC_REFLECT( _abi_hash, (owner)(hash) );

struct connector {
   asset balance;
   double weight = .5;
};
FC_REFLECT( connector, (balance)(weight) );

using namespace eosio_system;

BOOST_AUTO_TEST_SUITE(eosio_system_tests)

bool within_error(int64_t a, int64_t b, int64_t err) { return std::abs(a - b) <= err; };
bool within_one(int64_t a, int64_t b) { return within_error(a, b, 1); }

BOOST_FIXTURE_TEST_CASE( buysell, eosio_system_tester ) try {

   BOOST_REQUIRE_EQUAL( core_sym::from_string("0.0000"), get_balance( "alice1111111" ) );

   transfer( "eosio", "alice1111111", core_sym::from_string("1000.0000"), "eosio" );
   BOOST_REQUIRE_EQUAL( success(), stake( "eosio", "alice1111111", core_sym::from_string("200.0000"), core_sym::from_string("100.0000") ) );

   auto total = get_total_stake( "alice1111111" );
   auto init_bytes =  total["ram_bytes"].as_uint64();

   const a