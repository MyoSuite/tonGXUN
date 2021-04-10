---
content_title: RAM as system resource
---

## What is RAM

RAM is the memory, the storage space, where the blockchain stores data. If your contract needs to store data on the blockchain, like in a database, then it can store it in the blockchain's RAM using either a `multi-index table` or a `singleton`.

### Related documentation articles

- Multi-index table [explainer documentation page](https://github.com/AntelopeIO/cdt/blob/main/libraries/eosiolib/contracts/eosio/multi_index.hpp)

- Multi-index table [how to documentation page](https://github.com/AntelopeIO/cdt/tree/main/docs/06_how-to-guides/40_multi-index)

- Singleton [reference documentation page](https://github.com/AntelopeIO/cdt/blob/main/libraries/eosiolib/contracts/eosio/singleton.hpp) 

- Singleton [how to documentation page](https://github.com/AntelopeIO/cdt/blob/main/docs/06_how-to-guides/40_multi-index/how-to-define-a-singleton.md)

## RAM High Performance

The Antelope-based blockchains are known for their high performance, which is achieved also because the data stored on the blockchain is using RAM as the storage medium, and thus access to blockchain data is very fast, helping the performance benchmarks to reach levels no other blockchain has been able to.

## RAM Importance

RAM is a very important system resource because of the following reasons:

- It is a limited resource, each Antelope-based blockchain can have different policies and rules around RAM; for example the public EOS blo