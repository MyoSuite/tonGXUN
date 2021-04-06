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
      block_info::get_latest_block_batch_info(request.batch_start_height_offset, request.batch_size);

   response.result           = std::move(res.result);
   response.error_code.value = static_cast<uint32_t>(res.error_code);

   eosio::print("get_latest_block_batch_info: response error_code = ", response.error_code.value, "\n");
   if (response.result.has_value()) {
      const auto& result = *response.result;
      eosio::print("get_latest_block_batch_info: response result:\n");
      eosio::print("    batch_start_height          = ", result.batch_start_height, "\n");
      eosio::print("    batch_current_end_height    = ", result.batch_current_end_height, "\n");
   }

   return response;
}

output_type process_call(input_type input)
{
   return std::visit([](auto&& arg) -> output_type { return process(std::move(arg)); }, std::move(input));
}

} // namespace system_contracts::testing::test_contracts::blockinfo_tester

[[eosio::wasm_entry]] extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
   namespace ns = system_contracts::testing::test_contracts::blockinfo_tester;

   if (receiver == code) {
      if (ac