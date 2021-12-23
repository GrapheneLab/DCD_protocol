---
content_title: EOSIO Overview
---

EOSIO is the next-generation blockchain platform for creating and deploying smart contracts and distributed applications. EOSIO comes with a number of programs. The primary ones included in EOSIO are the following:

* [nodsig](01_nodsig/index.md) (node + sig = nodsig)  - core service daemon that runs a node for block production, API endpoints, or local development.
* [clsig](02_clsig/index.md) (cli + sig = clsig) - command line interface to interact with the blockchain (via `nodsig`) and manage wallets (via `ksigd`).
* [ksigd](03_ksigd/index.md) (key + sig = ksigd) - component that manages EOSIO keys in wallets and provides a secure enclave for digital signing.

The basic relationship between these components is illustrated in the diagram below.

![EOSIO components](eosio_components.png)

[[info | What's Next?]]
| [Install the EOSIO Software](00_install/index.md) before exploring the sections above.
