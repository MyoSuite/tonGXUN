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
   };

   /**
    * Weighted key.
    *
    * @details A weighted key is defined by a public key and an associated weight.
    */
   struct key_weight {
      eosio::public_key  key;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   /**
    * Wait weight.
    *
    * @details A wait weight is defined by a number of seconds to wait for and a weight.
    */
   struct wait_weight {
      uint32_t           wait_sec;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
   };

   /**
    * Blockchain authority.
    *
    * @details An authority is defined by:
    * - a vector of key_weights (a key_weight is a public key plus a weight),
    * - a vector of permission_level_weights, (a permission_level is an account name plus a permission name)
    * - a vector of wait_weights (a wait_weight is defined by a number of seconds to wait and a weight)
    * - a threshold value
    */
   struct authority {
      uint32_t                              threshold = 0;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;
      std::vector<wait_weight>              waits;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
   };

   /**
    * @defgroup eosioboot eosio.boot
    * @ingroup eosiocontracts
    *
    * eosio.boot is a extremely minimalistic system contract that only supports the native actions and an
    * activate action that allows activating desired protocol features prior to deploying a system contract
    * with more features such as eosio.bios or eosio.system.
    *
    * @{
    */
   class [[eosio::contract("eosio.boot")]] boot : public eosio::contract {
      public:
         using contract::contract;
         /**
          * @{
          * These actions map one-on-one with the ones defined in
          * [Native Action Handlers](@ref native_action_handlers) section.
          * They are present here so they can show up in the abi file and thus user can send them
          * to this contract, but they have no specific implementation at this contract level,
          * they will execute the implementation at the core level and nothing else.
          */
         /**
          * New account action
          *
          * @details Creates a new account.
          *
          * @param creator - the creator of the account
          * @param name - the name of the new account
          * @param owner - the authority for the owner permission of the new account
          * @param active - the authority for the active permission of the new account
          */
         [[eosio::action]]
         void newaccount( name             creator,
                          name             name,
                          ignore<authority> owner,
                          ignore<authority> active) {}
         /**
          * Update authorization action.
          *
          * @details Updates pemission for an account.
          *
          * @param a