#pragma once

#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>

namespace eosioboot {

   using eosio::action_wrapper;
   using eosio::check;
   using eosio::checksum256;
   using eosio::ignore;
   using eosio::name;
   using eosio::permission_level;
   using eosio::public_key;

   /**
    * A weighted permission.
    *
    * @details Defines a weighted permission, that is a permission which has a weight associated.
    * A permission is defined by an account name plus a permission name. The weight is going to be
    * used against a threshold, if the weight is equal or greater than the threshold set then authorization
    * will pass.
    */
   struct permission_level_weight {
      permission_level  permission;
      uint16_t          weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
 