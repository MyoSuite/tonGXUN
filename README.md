# Reference contracts

Reference contracts are a collection of contracts deployable to an [Antelope](https://github.com/AntelopeIO) blockchain which implements a lot of critical functionality that goes beyond what is provided by the base Antelope protocol.

The Antelope protocol includes capabilities such as:
* an accounts and permissions system which enables a flexible permission system that allows authorization authority over specific actions in a transaction to be satisfied by the appropriate combination of signatures;
* a consensus algorithm to propose and finalize blocks by a set of active block producers that can be arbitrarily selected by privileged smart contracts running on the blockchain;
* a basic resource management system that tracks usage of CPU/NET and RAM per account and enforces limits based on per-account quotas that can be adjusted by privileged smart contracts.

However, the Antelope protocol itself does not immediately provide:
* a mechanism for multiple accounts to reach consensus on authorization of a proposed transaction on-chain before executing it;
* a consensus mechanism that goes beyond the consensus algorithm to determine how block producers are selected and to align incentives by providing appropriate rewards and punishments to block producers or the entities that get them into that position;
* more sophisticated resource management systems that create markets for users to acquire resource rights;
* or, even something as seemingly basic as the concept of tokens (whether fungible or non-fungible).

The reference contracts in this repository provide all of the above and more by building higher-level features or abstractions on top of the primitive mechanisms provided by the Antelope protocol.

The collection of reference contracts consists of the following individual contracts:

* [boot contract](contracts/eosio.boot/include/eosio.boot/eosio.boot.hpp): A minimal contract that only serves the purpose of activating