
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include <eosio/crypto.hpp>
#include <eosio/dispatcher.hpp>

#include <cmath>

namespace eosiosystem {

   using eosio::current_time_point;
   using eosio::token;

   double get_continuous_rate(int64_t annual_rate) {
      return std::log1p(double(annual_rate)/double(100*inflation_precision));
   }
