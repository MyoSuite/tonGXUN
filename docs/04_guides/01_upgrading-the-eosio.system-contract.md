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
code hash: cc0ffc30150a07c487d8247a484ce1caf9c95779521d