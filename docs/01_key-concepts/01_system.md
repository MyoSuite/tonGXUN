---
content_title: System contracts, system accounts, privileged accounts
---

At the genesis of an Antelope-based blockchain, there is only one account present, `eosio` account, which is the main `system account`. There are other `system account`s, created by `eosio` account, which control specific actions of the `system contract`s [mentioned in previous section](../#system-contracts-defined-in-reference-contracts). __Note__ the terms `system contract` and `system account`. `Privileged accounts` are accounts which can execute a transaction while skipping the standard authorization check. To ensure that this is not a security hole, the permission authority over these accounts is granted to `eosio.prods` system account.

As you just learned the relation between a `system account` and a `system contract`, it is also important to remember that not all system accounts contain a system contract, but each system account has important roles in the blockc