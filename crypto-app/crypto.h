// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "wolfssl/wolfssl/wolfcrypt/aes.h" // https://github.com/wolfSSL/wolfssl/blob/master/doc/dox_comments/header_files/aes.h

int encrypt_message(const char message[], char cypher[], uint64_t message_len);
int decrypt_message(const char cypher[], char plaintext[], uint64_t message_len);