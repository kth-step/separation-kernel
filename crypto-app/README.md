# A simple crypt o app
This is a simple crypto app that receives messages and either encrypts them or decrypts them with its own private key. The encryption used is AES in CBC mode, with a 128 bit key. 

NOTE: the purpose of this app is only to simulate message passing utilizing AES encryption and decryption. It gives no guarantess that it follows best practices regarding security and it should not be used for any real encryption/decryption purposes.

This app makes use of the AES encryption implementation of [wolfSSL](https://github.com/wolfSSL/wolfssl). All files under the "wolfssl" directory are from the [wolfSSL](https://github.com/wolfSSL/wolfssl) repo, specifically as found in commit "802e3127c0455afd1d1b5f8d1eb72ab7ea09e80a". This app takes no credit for said files and for licensing regarding these files, see the [wolfSSL](https://github.com/wolfSSL/wolfssl) repo.