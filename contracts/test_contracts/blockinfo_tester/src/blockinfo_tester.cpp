#include <blockinfo_tester/blockinfo_tester.hpp>

#include <type_traits>
#include <vector>

#include <eosio/action.hpp>
#include <eosio/check.hpp>
#include <eosio/datastream.hpp>
#include <eosio/name.hpp>
#include <eosio/print.hpp>

namespace {

namespace block_info = eosiosystem::block_info;

}
namespace system_contracts::testing::test_contracts::blockinfo_tester {

auto process(get_latest_block_batch_info request) -> latest_block_batch_info_result
{
   latest_block_batch_info_result response;

   block_info::latest_block_batch_info_result res =
      block_info::get_latest_block_batch_info(request.batch_s