---
content_title: Staking on Antelope-based blockchains
---

## System Resources

Antelope-based blockchains work with three system resources:

* [RAM](02_ram.md)
* [CPU](03_cpu.md)
* [NET](04_net.md)

## How To Allocate System Resources

Antelope-based blockchain accounts need sufficient system resources, RAM, CPU and NET, to interact with the smart contracts deployed on the blockchain.

### Stake NET and CPU

The CPU and NET system resources are allocated by the account owner via the staking mechanism. Refer to the [cleos manual](https://github.com/AntelopeIO/leap/blob/main/docs/02_cleos/02_how-to-guides/how-to-stake-resource.md) on how to do it via the command line interface.

You will also find that staking/unstaking is at times referred to as delegating/undelegating. The economics of staking is also to provably commit to a promise that you will hold the staked tokens, either for NET or CPU, for a pre-established period of time, in spite of inflation caused by minting new tokens in order to reward BPs for their services every 24 hours.

When you stake tokens for CPU and NET, you gain access to system resources proportional to the total amount of tokens staked by all other users for the same system resource at the same time. This means you can execute transactions at no cost but in the limits of the staked tokens. The staked tokens guarantee the proportional amount of resources regardless of any variations in the free market.

If an account consumes all its allocated CPU and NET resources, it has two options:

* It can wait for the blockchain to replenish the consumed resources. You can read more details below about the [system resources replenish algorithm].(05_stake.md#System-Resources-Replenish-Algorithm).
* It can allocate more resources through the staking mechanism.

When an account uses the allocated resources, the amount that can be used in one transaction is limited by predefine [maximum CPU](https://github.com/AntelopeIO/cdt/blob/main/libraries/eosiolib/contracts/eosio/privileged.hpp#L110), [minimum CPU](https://github.com/AntelopeIO/cdt/blob/main/libraries/eosiolib/contracts/eosio/privileged.hpp#L116), and [maximum NET](https://github.com/AntelopeIO/cdt/blob/main/libraries/eosiolib/contracts/eosio/privileged.hpp#L69) limits. Transactions executed by the blockchain contain one or more actions, and each transaction must consume an amount of CPU and NET which is in the limits defined by the aforementioned blockchain settings.

#### System Resources Replenish Algorithm

Antelope-based blockchains replenish automatically the consumed CPU and NET system resources. Before a transaction is executed, by the blockchain, it first calculates how many resources the account executing the transaction can consume. The calculation uses an exponential moving average with linear extrapolation when data is missing, and it multiplies the currently accumulated average by `(number of blocks in the window - number of blocks since last update) / (number of blocks in the window)`. The window is set as 24 hours window.

This formula has