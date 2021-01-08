#pragma once

#include <eosio/action.hpp>
#include <eosio/binary_extension.hpp>
#include <eosio/contract.hpp>
#include <eosio/crypto.hpp>
#include <eosio/fixed_bytes.hpp>
#include <eosio/ignore.hpp>
#include <eosio/print.hpp>
#include <eosio/privileged.hpp>
#include <eosio/producer_schedule.hpp>

namespace eosiosystem {

   using eosio::binary_extension;
   using eosio::checksum256;
   using eosio::ignore;
   using eosio::name;
   using eosio::permission_level;
   using eosio::public_key;

   // TELOS BEGIN
   struct producer_metric {
      name bp_name;
      uint32_t missed_blocks_per_cycle = 12;

      // explicit serialization macro is not necessary, used here only to improve
      // compilation time
      EOSLIB_SERIALIZE(producer_metric, (bp_name)(missed_blocks_per_cycle))
   };
   // TELOS END

   /**
    * A weighted permission.
    *
    * Defines a weighted permission, that is a permission which has a weight associated.
    * A permission is defined by an account name plus a permission name.
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
    * A weighted key is defined by a public key and an associated weight.
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
    * A wait weight is defined by a number of seconds to wait for and a weight.
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
    * An authority is defined by:
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
    * Blockchain block header.
    *
    * A block header is defined by:
    * - a timestamp,
    * - the producer that created it,
    * - a confirmed flag default as zero,
    * - a link to previous block,
    * - a link to the transaction merkel root,
    * - a link to action root,
    * - a schedule version,
    * - and a producers' schedule.
    */
   struct block_header {
      uint32_t                                  timestamp;
      name                                      producer;
      uint16_t                                  confirmed = 0;
      checksum256                               previous;
      checksum256                               transaction_mroot;
      checksum256                               action_mroot;
      uint32_t                                  schedule_version = 0;
      std::optional<eosio::producer_schedule>   new_producers;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
                                     (schedule_version)(new_producers))
   };

   /**
    * abi_hash is the structure underlying the abihash table and consists of:
    * - `owner`: the account owner of the contract's abi
    * - `hash`: is the sha256 hash of the abi/binary
    */
   struct [[eosio::table("abihash"), eosio::contract("eosio.system")]] abi_hash {
      name              owner;
      checksum256       hash;
      uint64_t primary_key()const { return owner.value; }

      EOSLIB_SERIALIZE( abi_hash, (owner)(hash) )
   };

   void check_auth_change(name contract, name account, const binary_extension<name>& authorized_by);

   // Method parameters commented out to prevent generation of code that parses input data.
   /**
    * The EOSIO core `native` contract that governs authorization and contracts' abi.
    */
   class [[eosio::contract("eosio.system")]] native : public eosio::contract {
      public:

         using eosio::contract::contract;

         /**
          * These actions map one-on-one with the ones defined in core layer of EOSIO, that's where their implementation
          * actually is done.
          * They are present here only so they can show up in the abi file and thus user can send them
          * to this contract, but they have no specific implementation at this contract level,
          * they will execute the implementation at the core layer and nothing else.
          */

         /**
          * New account action is called after a new account is created. This code enforces resource-limits rules
          * for new accounts as well as new account naming conventions.
          */
         [[eosio::action]]
         void newaccount( const name&       creator,
                          const name&       name,
                          ignore<authority> owner,
                          ignore<authority> active);

         /**
          * Update authorization action updates permission for an account.
          *
          * This contract enforces additional rules:
          *
          * 1. If authorized_by is present and not "", then the contract does a
          *    require_auth2(account, authorized_by).
          * 2. If the account has opted into limitauthchg, then authorized_by
          *    must be present and not "".
          * 3. If the account has opted into limitauthchg, and allow_perms is
          *    not empty, then authorized_by must be in the array.
          * 4. If the account has o