# Reference contracts

Reference contracts are a collection of contracts deployable to an [Antelope](https://github.com/AntelopeIO) blockchain which implements a lot of critical functionality that goes beyond what is provided by the base Antelope protocol.

The Antelope protocol includes capabilities such as:
* an accounts and permissions system which enables a flexible permission system that allows authorization authority over specific actions in a transaction to be satisfied by the appropriate combination of signatures;
* a consensus algorithm to propose and finalize blocks by a set of active block producers that can be arbitrarily selected by privileged smart contracts running on the blockchain;
* a basic resource management system that tracks usage of CPU/NET and RAM per account and enforces limits based on per-account quotas that can be adjusted by p