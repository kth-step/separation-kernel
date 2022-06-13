// See LICENSE file for copyright and license details.
#pragma once

#include "wolfssl/wolfssl/wolfcrypt/aes.h" // https://github.com/wolfSSL/wolfssl/blob/master/doc/dox_comments/header_files/aes.h

int encrypt_message(const byte message[], byte cypher[], int message_len);
int decrypt_message(const byte cypher[], byte plaintext[], int message_len);