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

   const asset initial_ram_balance = get_balance("eosio.ram"_n);
   const asset initial_ramfee_balance = get_balance("eosio.ramfee"_n);
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("200.0000") ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("800.0000"), get_balance( "alice1111111" ) );
   BOOST_REQUIRE_EQUAL( initial_ram_balance + core_sym::from_string("199.0000"), get_balance("eosio.ram"_n) );
   BOOST_REQUIRE_EQUAL( initial_ramfee_balance + core_sym::from_string("1.0000"), get_balance("eosio.ramfee"_n) );

   total = get_total_stake( "alice1111111" );
   auto bytes = total["ram_bytes"].as_uint64();
   auto bought_bytes = bytes - init_bytes;
   wdump((init_bytes)(bought_bytes)(bytes) );

   BOOST_REQUIRE_EQUAL( true, 0 < bought_bytes );

   BOOST_REQUIRE_EQUAL( success(), sellram( "alice1111111", bought_bytes ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("998.0049"), get_balance( "alice1111111" ) );
   total = get_total_stake( "alice1111111" );
   BOOST_REQUIRE_EQUAL( true, total["ram_bytes"].as_uint64() == init_bytes );

   transfer( "eosio", "alice1111111", core_sym::from_string("100000000.0000"), "eosio" );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("100000998.0049"), get_balance( "alice1111111" ) );
   // alice buys ram for 10000000.0000, 0.5% = 50000.0000 go to ramfee
   // after fee 9950000.0000 go to bought bytes
   // when selling back bought bytes, pay 0.5% fee and get back 99.5% of 9950000.0000 = 9900250.0000
   // expected account after that is 90000998.0049 + 9900250.0000 = 99901248.0049 with a difference
   // of order 0.0001 due to rounding errors
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10000000.0000") ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("90000998.0049"), get_balance( "alice1111111" ) );

   total = get_total_stake( "alice1111111" );
   bytes = total["ram_bytes"].as_uint64();
   bought_bytes = bytes - init_bytes;
   wdump((init_bytes)(bought_bytes)(bytes) );

   BOOST_REQUIRE_EQUAL( success(), sellram( "alice1111111", bought_bytes ) );
   total = get_total_stake( "alice1111111" );

   bytes = total["ram_bytes"].as_uint64();
   bought_bytes = bytes - init_bytes;
   wdump((init_bytes)(bought_bytes)(bytes) );

   BOOST_REQUIRE_EQUAL( true, total["ram_bytes"].as_uint64() == init_bytes );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("99901248.0040"), get_balance( "alice1111111" ) );  // TELOS

   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("100.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("100.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("100.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("100.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("100.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("30.0000") ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("99900688.0040"), get_balance( "alice1111111" ) );  // TELOS

   auto newtotal = get_total_stake( "alice1111111" );

   auto newbytes = newtotal["ram_bytes"].as_uint64();
   bought_bytes = newbytes - bytes;
   wdump((newbytes)(bytes)(bought_bytes) );

   BOOST_REQUIRE_EQUAL( success(), sellram( "alice1111111", bought_bytes ) );
   BOOST_REQUIRE_EQUAL( core_sym::from_string("99901242.4175"), get_balance( "alice1111111" ) );  // TELOS

   newtotal = get_total_stake( "alice1111111" );
   auto startbytes = newtotal["ram_bytes"].as_uint64();

   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10000000.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10000000.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10000000.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram( "alice1111111", "alice1111111", core_sym::from_string("10000000.0000") ) );
   BOOST_REQUIRE_EQUAL( success(), buyram