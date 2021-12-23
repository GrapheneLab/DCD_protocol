---
content_title: ksigd FAQ
---

### How does `ksigd` store key pairs

`ksigd` encrypts key pairs under-the-hood before storing them on a wallet file. Depending on the wallet implementation, say Secure Clave or YubiHSM, a specific cryptographic algorithm will be used. When the standard file system of a UNIX-based OS is used, `ksigd` encrypts key pairs using 256-bit AES in CBC mode.

### How to enable the `ksigd` Secure Enclave

To enable the secure enclave feature of `ksigd`, you need to sign a `ksigd` binary with a certificate provided with your Apple Developer Account. Be aware that there might be some constraints imposed by App Store when signing from a console application. Therefore, the signed binaries might need to be resigned every 7 days.
