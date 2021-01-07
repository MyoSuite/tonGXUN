#pragma once

#include <eosio/multi_index.hpp>

namespace eosiosystem {
   using eosio::name;

   struct [[eosio::table("limitauthchg"),eosio::contract("eosio.system")]] limit_auth_change {
      uint8_t              version = 0;
      name                 account;
      std