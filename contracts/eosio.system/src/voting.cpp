#include <eosio/crypto.hpp>
#include <eosio/datastream.hpp>
#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/permission.hpp>
#include <eosio/privileged.hpp>
#include <eosio/serialize.hpp>
#include <eosio/singleton.hpp>

#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <type_traits>
#include <limits>
#include <set>
#include <algorithm>
#include <cmath>

// TELOS BEGIN
#include <boost/container/flat_map.hpp>
#include "system_rotation.cpp"
// TELOS END

namespace eosiosystem {

   using eosio::const_mem_fun;
   using eosio::current_time_point;
   using eosio::indexed_by;
   using eosio::microseconds;
   using eosio::singleton;

   void system_contract::register_producer( const name& producer, const eosio::block_signing_authority& producer_authority, const std::string& url, uint16_t location ) {
      auto prod = _producers.find( producer.value );
      const auto ct = current_time_point();

      eosio::public_key producer_key{};

      std::visit( [&](auto&& auth ) {
         if( auth.keys.size() == 1 ) {
            // if the producer_authority consists of a single key, use that key in the legacy producer_key field
            producer_key = auth.keys[0].key;
         }
      }, producer_authority );

      if ( prod != _producers.end() ) {
         _producers.modify( prod, producer, [&]( producer_info& info ){
            info.producer_key       = producer_key;
            info.is_active          = true;
            info.url               