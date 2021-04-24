
#pragma once

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/resource_limits.hpp>
#include "contracts.hpp"
#include "test_symbol.hpp"

#include <fc/variant_object.hpp>
#include <fstream>

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

#ifndef TESTER
#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif
#endif

namespace eosio_system {

auto dump_trace = [](transaction_trace_ptr trace_ptr) -> transaction_trace_ptr {
   std::cout << std::endl << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
   for(auto trace : trace_ptr->action_traces) {
      std::cout << "action_name trace: " << trace.act.name.to_string() << std::endl;
      //TODO: split by new line character, loop and indent output
      std::cout << trace.console << std::endl << std::endl;
   }
   std::cout << std::endl << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl << std::endl;
   return trace_ptr;
};

class eosio_system_tester : public TESTER {
public:

   void basic_setup() {
      produce_blocks( 2 );

      create_accounts({ "eosio.token"_n, "eosio.ram"_n, "eosio.ramfee"_n, "eosio.stake"_n,
               "eosio.bpay"_n, "eosio.vpay"_n, "eosio.saving"_n, "eosio.names"_n, "eosio.rex"_n,
               // TELOS BEGIN
               "exrsrv.tf"_n, "telos.decide"_n, "works.decide"_n
               // TELOS END
               });


      produce_blocks( 100 );
      set_code( "eosio.token"_n, contracts::token_wasm());
      set_abi( "eosio.token"_n, contracts::token_abi().data() );
      {
         const auto& accnt = control->db().get<account_object,by_name>( "eosio.token"_n );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         token_abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));
      }
   }

   void create_core_token( symbol core_symbol = symbol{CORE_SYM} ) {
      FC_ASSERT( core_symbol.decimals() == 4, "create_core_token assumes core token has 4 digits of precision" );
      create_currency( "eosio.token"_n, config::system_account_name, asset(100000000000000, core_symbol) );
      issue( asset(10000000000000, core_symbol) );
      BOOST_REQUIRE_EQUAL( asset(10000000000000, core_symbol), get_balance( "eosio", core_symbol ) );
   }

   void deploy_contract( bool call_init = true ) {
      set_code( config::system_account_name, contracts::system_wasm() );
      set_abi( config::system_account_name, contracts::system_abi().data() );
      if( call_init ) {
         base_tester::push_action(config::system_account_name, "init"_n,
                                               config::system_account_name,  mutable_variant_object()
                                               ("version", 0)
                                               ("core", CORE_SYM_STR)
         );
      }

      {
         const auto& accnt = control->db().get<account_object,by_name>( config::system_account_name );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));
      }

      set_code( "telos.decide"_n, contracts::decide_wasm() );
      set_abi( "telos.decide"_n, contracts::decide_abi().data() );

      {
         const auto& accnt = control->db().get<account_object,by_name>( "telos.decide"_n );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         decide_abi_ser.set_abi(abi, abi_serializer::create_yield_function(abi_serializer_max_time));
      }
   }

   void remaining_setup() {
      produce_blocks();

      // Assumes previous setup steps were done with core token symbol set to CORE_SYM
      create_account_with_resources( "alice1111111"_n, config::system_account_name, core_sym::from_string("1.0000"), false );
      create_account_with_resources( "bob111111111"_n, config::system_account_name, core_sym::from_string("0.4500"), false );
      create_account_with_resources( "carol1111111"_n, config::system_account_name, core_sym::from_string("1.0000"), false );

      BOOST_REQUIRE_EQUAL( core_sym::from_string("1000000000.0000"), get_balance("eosio")  + get_balance("eosio.ramfee") + get_balance("eosio.stake") + get_balance("eosio.ram") );
      // TELOS BEGIN
      produce_blocks();
      open( "exrsrv.tf"_n, symbol{CORE_SYM}, "exrsrv.tf"_n );
      // TELOS END
   }

   enum class setup_level {
      none,
      minimal,
      core_token,
      deploy_contract,
      full
   };

   eosio_system_tester( setup_level l = setup_level::full ) {
      if( l == setup_level::none ) return;

      basic_setup();
      if( l == setup_level::minimal ) return;

      create_core_token();
      if( l == setup_level::core_token ) return;

      deploy_contract();
      if( l == setup_level::deploy_contract ) return;

      remaining_setup();
   }

   template<typename Lambda>
   eosio_system_tester(Lambda setup) {
      setup(*this);

      basic_setup();
      create_core_token();
      deploy_contract();
      remaining_setup();
   }


   void create_accounts_with_resources( vector<account_name> accounts, account_name creator = config::system_account_name ) {
      for( auto a : accounts ) {
         create_account_with_resources( a, creator );
      }
   }

   transaction_trace_ptr create_account_with_resources( account_name a, account_name creator, uint32_t ram_bytes = 8000 ) {
      signed_transaction trx;
      set_transaction_headers(trx);

      authority owner_auth;
      owner_auth =  authority( get_public_key( a, "owner" ) );

      trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
                                newaccount{
                                   .creator  = creator,
                                   .name     = a,
                                   .owner    = owner_auth,
                                   .active   = authority( get_public_key( a, "active" ) )
                                });

      trx.actions.emplace_back( get_action( config::system_account_name, "buyrambytes"_n, vector<permission_level>{{creator,config::active_name}},
                                            mvo()
                                            ("payer", creator)
                                            ("receiver", a)
                                            ("bytes", ram_bytes) )
                              );
      trx.actions.emplace_back( get_action( config::system_account_name, "delegatebw"_n, vector<permission_level>{{creator,config::active_name}},
                                            mvo()
                                            ("from", creator)
                                            ("receiver", a)
                                            ("stake_net_quantity", core_sym::from_string("10.0000") )
                                            ("stake_cpu_quantity", core_sym::from_string("10.0000") )
                                            ("transfer", 0 )
                                          )
                                );

      set_transaction_headers(trx);
      trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
      return push_transaction( trx );
   }

   // TELOS BEGIN
   transaction_trace_ptr on_block() {
      signed_transaction trx;
      set_transaction_headers(trx);
      action on_block_act;
      on_block_act.account = config::system_account_name;
      on_block_act.name = "onblock"_n;
      on_block_act.authorization = vector<permission_level>{{config::system_account_name, config::active_name}};
      on_block_act.data = fc::raw::pack(control->head_block_header());
      trx.actions.emplace_back(on_block_act);
      trx.sign( get_private_key( config::system_account_name, "active" ), control->get_chain_id()  );
      auto t = push_transaction(trx);
      dump_trace(t);
      return t;
   }
   // TELOS END

   transaction_trace_ptr create_account_with_resources( account_name a, account_name creator, asset ramfunds, bool multisig,
                                                        asset net = core_sym::from_string("10.0000"), asset cpu = core_sym::from_string("10.0000") ) {
      signed_transaction trx;
      set_transaction_headers(trx);

      authority owner_auth;
      if (multisig) {
         // multisig between account's owner key and creators active permission
         owner_auth = authority(2, {key_weight{get_public_key( a, "owner" ), 1}}, {permission_level_weight{{creator, config::active_name}, 1}});
      } else {
         owner_auth =  authority( get_public_key( a, "owner" ) );
      }

      trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
                                newaccount{
                                   .creator  = creator,
                                   .name     = a,
                                   .owner    = owner_auth,
                                   .active   = authority( get_public_key( a, "active" ) )
                                });

      trx.actions.emplace_back( get_action( config::system_account_name, "buyram"_n, vector<permission_level>{{creator,config::active_name}},
                                            mvo()
                                            ("payer", creator)
                                            ("receiver", a)
                                            ("quant", ramfunds) )
                              );

      trx.actions.emplace_back( get_action( config::system_account_name, "delegatebw"_n, vector<permission_level>{{creator,config::active_name}},
                                            mvo()
                                            ("from", creator)
                                            ("receiver", a)
                                            ("stake_net_quantity", net )
                                            ("stake_cpu_quantity", cpu )
                                            ("transfer", 0 )
                                          )
                                );

      set_transaction_headers(trx);
      trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
      return push_transaction( trx );
   }

   transaction_trace_ptr setup_producer_accounts( const std::vector<account_name>& accounts,
                                                  asset ram = core_sym::from_string("1.0000"),
                                                  asset cpu = core_sym::from_string("80.0000"),
                                                  asset net = core_sym::from_string("80.0000")
                                                )
   {
      account_name creator(config::system_account_name);
      signed_transaction trx;
      set_transaction_headers(trx);

      for (const auto& a: accounts) {
         authority owner_auth( get_public_key( a, "owner" ) );
         trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
                                   newaccount{
                                         .creator  = creator,
                                         .name     = a,
                                         .owner    = owner_auth,
                                         .active   = authority( get_public_key( a, "active" ) )
                                         });

         trx.actions.emplace_back( get_action( config::system_account_name, "buyram"_n, vector<permission_level>{ {creator, config::active_name} },
                                               mvo()
                                               ("payer", creator)
                                               ("receiver", a)
                                               ("quant", ram) )
                                   );

         trx.actions.emplace_back( get_action( config::system_account_name, "delegatebw"_n, vector<permission_level>{ {creator, config::active_name} },
                                               mvo()
                                               ("from", creator)
                                               ("receiver", a)
                                               ("stake_net_quantity", net)
                                               ("stake_cpu_quantity", cpu )
                                               ("transfer", 0 )
                                               )
                                   );
      }

      set_transaction_headers(trx);
      trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
      return push_transaction( trx );
   }

   action_result buyram( const account_name& payer, account_name receiver, const asset& eosin ) {
      return push_action( payer, "buyram"_n, mvo()( "payer",payer)("receiver",receiver)("quant",eosin) );
   }
   action_result buyram( std::string_view payer, std::string_view receiver, const asset& eosin ) {
      return buyram( account_name(payer), account_name(receiver), eosin );
   }

   action_result buyrambytes( const account_name& payer, account_name receiver, uint32_t numbytes ) {
      return push_action( payer, "buyrambytes"_n, mvo()( "payer",payer)("receiver",receiver)("bytes",numbytes) );
   }
   action_result buyrambytes( std::string_view payer, std::string_view receiver, uint32_t numbytes ) {
      return buyrambytes( account_name(payer), account_name(receiver), numbytes );
   }

   action_result sellram( const account_name& account, uint64_t numbytes ) {
      return push_action( account, "sellram"_n, mvo()( "account", account)("bytes",numbytes) );
   }
   action_result sellram( std::string_view account, uint64_t numbytes ) {
      return sellram( account_name(account), numbytes );
   }

   action_result push_action( const account_name& signer, const action_name &name, const variant_object &data, bool auth = true ) {
         string action_type_name = abi_ser.get_action_type(name);

         action act;
         act.account = config::system_account_name;
         act.name = name;
         act.data = abi_ser.variant_to_binary( action_type_name, data, abi_serializer::create_yield_function(abi_serializer_max_time) );

         return base_tester::push_action( std::move(act), (auth ? signer : signer == "bob111111111"_n ? "alice1111111"_n : "bob111111111"_n).to_uint64_t() );
   }

   action_result stake( const account_name& from, const account_name& to, const asset& net, const asset& cpu ) {
      return push_action( name(from), "delegatebw"_n, mvo()
                          ("from",     from)
                          ("receiver", to)
                          ("stake_net_quantity", net)
                          ("stake_cpu_quantity", cpu)
                          ("transfer", 0 )
      );
   }
   action_result stake( std::string_view from, std::string_view to, const asset& net, const asset& cpu ) {
      return stake( account_name(from), account_name(to), net, cpu );
   }

   action_result stake( const account_name& acnt, const asset& net, const asset& cpu ) {
      return stake( acnt, acnt, net, cpu );
   }
   action_result stake( std::string_view acnt, const asset& net, const asset& cpu ) {
      return stake( account_name(acnt), net, cpu );
   }

   action_result stake_with_transfer( const account_name& from, const account_name& to, const asset& net, const asset& cpu ) {
      return push_action( name(from), "delegatebw"_n, mvo()
                          ("from",     from)
                          ("receiver", to)
                          ("stake_net_quantity", net)
                          ("stake_cpu_quantity", cpu)
                          ("transfer", true )
      );
   }
   action_result stake_with_transfer( std::string_view from, std::string_view to, const asset& net, const asset& cpu ) {
      return stake_with_transfer( account_name(from), account_name(to), net, cpu );
   }

   action_result stake_with_transfer( const account_name& acnt, const asset& net, const asset& cpu ) {
      return stake_with_transfer( acnt, acnt, net, cpu );
   }
   action_result stake_with_transfer( std::string_view acnt, const asset& net, const asset& cpu ) {
      return stake_with_transfer( account_name(acnt), net, cpu );
   }

   action_result unstake( const account_name& from, const account_name& to, const asset& net, const asset& cpu ) {
      return push_action( name(from), "undelegatebw"_n, mvo()
                          ("from",     from)
                          ("receiver", to)
                          ("unstake_net_quantity", net)
                          ("unstake_cpu_quantity", cpu)
      );
   }
   action_result unstake( std::string_view from, std::string_view to, const asset& net, const asset& cpu ) {
      return unstake( account_name(from), account_name(to), net, cpu );
   }

   action_result unstake( const account_name& acnt, const asset& net, const asset& cpu ) {
      return unstake( acnt, acnt, net, cpu );
   }
   action_result unstake( std::string_view acnt, const asset& net, const asset& cpu ) {
      return unstake( account_name(acnt), net, cpu );
   }

   int64_t bancor_convert( int64_t S, int64_t R, int64_t T ) { return double(R) * T  / ( double(S) + T ); };

   int64_t get_net_limit( account_name a ) {
      int64_t ram_bytes = 0, net = 0, cpu = 0;
      control->get_resource_limits_manager().get_account_limits( a, ram_bytes, net, cpu );
      return net;
   };

   int64_t get_cpu_limit( account_name a ) {
      int64_t ram_bytes = 0, net = 0, cpu = 0;
      control->get_resource_limits_manager().get_account_limits( a, ram_bytes, net, cpu );
      return cpu;
   };

   action_result deposit( const account_name& owner, const asset& amount ) {
      return push_action( name(owner), "deposit"_n, mvo()
                          ("owner",  owner)
                          ("amount", amount)
      );
   }

   action_result withdraw( const account_name& owner, const asset& amount ) {
      return push_action( name(owner), "withdraw"_n, mvo()
                          ("owner",  owner)
                          ("amount", amount)
      );
   }

   action_result buyrex( const account_name& from, const asset& amount ) {
      return push_action( name(from), "buyrex"_n, mvo()
                          ("from",   from)
                          ("amount", amount)
      );
   }

   asset get_buyrex_result( const account_name& from, const asset& amount ) {
      auto trace = base_tester::push_action( config::system_account_name, "buyrex"_n, from, mvo()("from", from)("amount", amount) );
      asset rex_received;
      for ( size_t i = 0; i < trace->action_traces.size(); ++i ) {
         if ( trace->action_traces[i].act.name == "buyresult"_n ) {
            fc::raw::unpack( trace->action_traces[i].act.data.data(),
                             trace->action_traces[i].act.data.size(),
                             rex_received );
            return rex_received;
         }
      }
      return rex_received;
   }

   action_result unstaketorex( const account_name& owner, const account_name& receiver, const asset& from_net, const asset& from_cpu ) {
      return push_action( name(owner), "unstaketorex"_n, mvo()
                          ("owner",    owner)
                          ("receiver", receiver)
                          ("from_net", from_net)
                          ("from_cpu", from_cpu)
      );
   }

   asset get_unstaketorex_result( const account_name& owner, const account_name& receiver, const asset& from_net, const asset& from_cpu ) {
      auto trace = base_tester::push_action( config::system_account_name, "unstaketorex"_n, owner, mvo()
                                             ("owner", owner)
                                             ("receiver", receiver)
                                             ("from_net", from_net)
                                             ("from_cpu", from_cpu)
      );
      asset rex_received;
      for ( size_t i = 0; i < trace->action_traces.size(); ++i ) {
         if ( trace->action_traces[i].act.name == "buyresult"_n ) {
            fc::raw::unpack( trace->action_traces[i].act.data.data(),
                             trace->action_traces[i].act.data.size(),
                             rex_received );
            return rex_received;
         }
      }
      return rex_received;
   }

   action_result sellrex( const account_name& from, const asset& rex ) {
      return push_action( name(from), "sellrex"_n, mvo()
                          ("from", from)
                          ("rex",  rex)
      );
   }

   asset get_sellrex_result( const account_name& from, const asset& rex ) {
      auto trace = base_tester::push_action( config::system_account_name, "sellrex"_n, from, mvo()("from", from)("rex", rex) );
      asset proceeds;
      for ( size_t i = 0; i < trace->action_traces.size(); ++i ) {
         if ( trace->action_traces[i].act.name == "sellresult"_n ) {
            fc::raw::unpack( trace->action_traces[i].act.data.data(),
                             trace->action_traces[i].act.data.size(),
                             proceeds );
            return proceeds;
         }
      }
      return proceeds;
   }

   auto get_rexorder_result( const transaction_trace_ptr& trace ) {
      std::vector<std::pair<account_name, asset>> output;
      for ( size_t i = 0; i < trace->action_traces.size(); ++i ) {
         if ( trace->action_traces[i].act.name == "orderresult"_n ) {
            fc::datastream<const char*> ds( trace->action_traces[i].act.data.data(),
                                            trace->action_traces[i].act.data.size() );
            account_name owner; fc::raw::unpack( ds, owner );
            asset proceeds; fc::raw::unpack( ds, proceeds );
            output.emplace_back( owner, proceeds );
         }
      }
      return output;
   }

   action_result cancelrexorder( const account_name& owner ) {
      return push_action( name(owner), "cnclrexorder"_n, mvo()("owner", owner) );
   }

   action_result rentcpu( const account_name& from, const account_name& receiver, const asset& payment, const asset& fund = core_sym::from_string("0.0000") ) {
      return push_action( name(from), "rentcpu"_n, mvo()
                          ("from",         from)
                          ("receiver",     receiver)
                          ("loan_payment", payment)
                          ("loan_fund",    fund)
      );
   }

   action_result rentnet( const account_name& from, const account_name& receiver, const asset& payment, const asset& fund = core_sym::from_string("0.0000") ) {
      return push_action( name(from), "rentnet"_n, mvo()
                          ("from",         from)
                          ("receiver",     receiver)