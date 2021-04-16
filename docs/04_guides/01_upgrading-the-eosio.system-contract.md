---
content_title: Upgrading the system contract
---

# Indirect method using eosio.msig contract

Cleos currently provides tools to propose an action with the eosio.msig contract, but it does not provide an easy interface to propose a custom transaction.

So, at the moment it is difficult to propose an atomic transaction with multiple actions (for example `eosio::setcode` followed by `eosio::setabi`).

The advantage of the eosio.msig method is that it makes coordination much easier and does not place strict time limits (less than 9 hours) on signature collection.

The disadvantage of the eosio.msig method is that it requires the proposer to have sufficient RAM to propose the transaction and currently cleos does not provide convenient tools to use it with custom transactions like the one that would be necessary to atomically upgrade the system contract.

For now, it is recommended to use the direct method to upgrade the system contract.

# Direct method (avoids using eosio.msig contract)

Each of the top 21 block producers should do the following:

1. Get current system contract for later comparison (actual hash and ABI on the main-net blockchain will be different):

```sh
cleos get code -c original_system_contract.wast -a original_system_contract.abi eosio
```
```console
code hash: cc0ffc30150a07c487d8247a484ce1caf9c95779521d8c230040c2cb0e2a3a60
saving wast to original_system_contract.wast
saving abi to original_system_contract.abi
```

2. Generate the unsigned transaction which upgrades the system contract:

```sh
cleos set contract -s -j -d eosio contracts/eosio.system | tail -n +4 > upgrade_system_contract_trx.json
```

The first few lines of the generated file should be something similar to (except with very different numbers for `expiration`, `ref_block_num`, and `ref_block_prefix`):

```json
{
   "expiration": "2018-06-15T22:17:10",
   "ref_block_num": 4552,
   "ref_block_prefix": 511016679,
   "max_net_usage_words": 0,
   "max_cpu_usage_ms": 0,
   "delay_sec": 0,
   "context_free_actions": [],
   "actions": [{
      "account": "eosio",
      "name": "setcode",
      "authorization": [{
          "actor": "eosio",
          "permission": "active"
        }
      ],
```

and the last few lines should be:

```json
      }
   ],
   "transaction_extensions": [],
   "signatures": [],
   "context_free_data": []
}
```

One of the top block producers should be chosen to lead the upgrade process. This lead producer should take their generated `upgrade_system_contract_trx.json`, rename it to `upgrade_system_contract_official_trx.json`, and do the following:

3. Modify the `expiration` timestamp in `upgrade_system_contract_official_trx.json` to a time that is sufficiently far in the future to give enough time to collect all the necessary signatures, but not more than 9 hours from the time the transaction was generated. Also, keep in mind that the transaction will not be accepted into the blockchain if the expiration is more than 1 hour from the present time.

4. Pass the `upgrade_system_contract_official_trx.json` file to all the other top 21 block producers.

Then each of the top 21 block producers should do the following:

5. Compare their generated `upgrade_system_contract_official_trx.json` file with the `upgrade_system_contract_official_trx.json` provided by the lead producer. The only difference should be in `expiration`, `ref_block_num`, `ref_block_prefix`, for example:

```sh
diff upgrade_system_contract_official_trx.json upgrade_system_contract_trx.json
```
```json
2,4c2,4
<   "expirati