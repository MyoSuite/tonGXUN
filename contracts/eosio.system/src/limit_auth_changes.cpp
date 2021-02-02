#include <eosio.system/limit_auth_changes.hpp>
#include <eosio.system/eosio.system.hpp>

namespace eosiosystem {

   void system_contract::limitauthchg(const name& account, const std::vector<name>& allow_perms,
                                      const std::vector<name>& disallow_perms) {
      limit_auth_change_table table(get_self(), get_self().value);
      require_auth(account);
      eosio::check(allow_perms.empty() || disallow_perms.empty(), "either allow_perms or disallow_perms must be empty