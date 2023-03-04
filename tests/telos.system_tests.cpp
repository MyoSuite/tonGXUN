
#include <boost/test/unit_test.hpp>

#include "eosio.system_tester.hpp"

#define MAX_PRODUCERS 42

using namespace eosio_system;

BOOST_AUTO_TEST_SUITE(telos_system_tests)

BOOST_FIXTURE_TEST_CASE(producer_onblock_check, eosio_system_tester) try {

   // min_activated_stake_part = 28'570'987'0000;
   const asset half_min_activated_stake = core_sym::from_string("14285493.5000");
   const asset quarter_min_activated_stake = core_sym::from_string("7142746.7500");
   const asset eight_min_activated_stake = core_sym::from_string("3571373.3750");

   const asset large_asset = core_sym::from_string("80.0000");
   create_account_with_resources( "producvotera"_n, config::system_account_name, core_sym::from_string("1.0000"), false, large_asset, large_asset );
   create_account_with_resources( "producvoterb"_n, config::system_account_name, core_sym::from_string("1.0000"), false, large_asset, large_asset );
   create_account_with_resources( "producvoterc"_n, config::system_account_name, core_sym::from_string("1.0000"), false, large_asset, large_asset );

   // create accounts {defproducera, defproducerb, ..., defproducerz} and register as producers
   std::vector<account_name> producer_names;
   producer_names.reserve('z' - 'a' + 1);
   const std::string root("defproducer");
   for ( char c = 'a'; c <= 'z'; ++c ) {
      producer_names.emplace_back(root + std::string(1, c));
   }
   setup_producer_accounts(producer_names);

   for (auto a:producer_names)
      regproducer(a);

   BOOST_REQUIRE_EQUAL(0, get_producer_info( producer_names.front() )["total_votes"].as<double>());
   BOOST_REQUIRE_EQUAL(0, get_producer_info( producer_names.back() )["total_votes"].as<double>());

   transfer(config::system_account_name, "producvotera", half_min_activated_stake, config::system_account_name);
   BOOST_REQUIRE_EQUAL(success(), stake("producvotera", quarter_min_activated_stake, quarter_min_activated_stake ));
   BOOST_REQUIRE_EQUAL(success(), vote( "producvotera"_n, vector<account_name>(producer_names.begin(), producer_names.begin()+10)));

   BOOST_CHECK_EQUAL( wasm_assert_msg( "cannot undelegate bandwidth until the chain is activated (1,000,000 blocks produced)" ),
                      unstake( "producvotera", core_sym::from_string("50.0000"), core_sym::from_string("50.0000") ) );

   // give a chance for everyone to produce blocks
   {
      produce_blocks(21 * 12);

      bool all_21_produced = true;
      for (uint32_t i = 0; i < 21; ++i) {
         if (0 == get_producer_info(producer_names[i])["unpaid_blocks"].as<uint32_t>()) {
            all_21_produced= false;
         }
      }
      bool rest_didnt_produce = true;
      for (uint32_t i = 21; i < producer_names.size(); ++i) {
         if (0 < get_producer_info(producer_names[i])["unpaid_blocks"].as<uint32_t>()) {
            rest_didnt_produce = false;
         }
      }
      BOOST_REQUIRE_EQUAL(false, all_21_produced);
      BOOST_REQUIRE_EQUAL(true, rest_didnt_produce);
   }

   {
      const char* claimrewards_activation_error_message = "cannot claim rewards until the chain is activated (1,000,000 blocks produced)";
      BOOST_CHECK_EQUAL(0, get_global_state()["total_unpaid_blocks"].as<uint32_t>());
      BOOST_REQUIRE_EQUAL(wasm_assert_msg( claimrewards_activation_error_message ),
                          push_action(producer_names.front(), "claimrewards"_n, mvo()("owner", producer_names.front())));
      BOOST_REQUIRE_EQUAL(0, get_balance(producer_names.front()).get_amount());
      BOOST_REQUIRE_EQUAL(wasm_assert_msg( claimrewards_activation_error_message ),
                          push_action(producer_names.back(), "claimrewards"_n, mvo()("owner", producer_names.back())));
      BOOST_REQUIRE_EQUAL(0, get_balance(producer_names.back()).get_amount());
   }


   // stake across 15% boundary
   transfer(config::system_account_name, "producvoterb", half_min_activated_stake, config::system_account_name);
   BOOST_REQUIRE_EQUAL(success(), stake("producvoterb", quarter_min_activated_stake, quarter_min_activated_stake));
   transfer(config::system_account_name, "producvoterc", quarter_min_activated_stake, config::system_account_name);
   BOOST_REQUIRE_EQUAL(success(), stake("producvoterc", eight_min_activated_stake, eight_min_activated_stake));

   // 21 => no rotation , 22 => yep, rotation
   BOOST_REQUIRE_EQUAL(success(), vote( "producvoterb"_n, vector<account_name>(producer_names.begin(), producer_names.begin()+22)));
   BOOST_REQUIRE_EQUAL(success(), vote( "producvoterc"_n, vector<account_name>(producer_names.begin(), producer_names.end())));

   activate_network();

   // give a chance for everyone to produce blocks
   {
      produce_blocks(21 * 12);
      // for (uint32_t i = 0; i < producer_names.size(); ++i) {
      //    std::cout<<"["<<producer_names[i]<<"]: "<<get_producer_info(producer_names[i])["unpaid_blocks"].as<uint32_t>()<<std::endl;
      // }