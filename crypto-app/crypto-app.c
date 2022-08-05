// See LICENSE file for copyright and license details.

#include "crypto-app.h"

#include <stdio.h>

#include "crypto.h"

/*
The _enc and _dec addr variables denote the beginning and end of the read and write areas for encryption and decryption respectively.
The read area is used as input for the operations, while the write is used for output. The end addresses are exclusive.

For simplicity the input and output memory regions must be of the same size.

The ready_input and ready_output variables denote the number of bytes from the begin address that have been written. E.g. if _enc_ready_input points
to the value 100, then [_enc_r_addr_begin, _enc_r_addr_begin + 100) denote the address range that may be read for input (addresses after have not been 
written by the provider of input and thus should not be read). 

When reaching the end of the address range (when addr_begin + ready_input/output is greater than addr_end) it will wrap around to the beginning, 
meaning ready_input/output will start counting from 0 again. If ready_input is ever smaller than ready_output then the available input has 
wrapped back like this while ready_output has yet to do it.

The protocol for the shared memory is as follows: when writing to the output area a number of size "message_size_byte_width" is written first to indicate 
how many bytes the actual message is. Immediately following this number is the message, taking up the number of bytes specified. The ready_output is then increased
by the size of the message + "message_size_byte_width". This value is always kept in modulo the size of the output area, and wraps back to 0 if needed. If ever
there is a situation where the remaining unwritten space of the output area is less than "message_size_byte_width" after a message has been written, ready_output is
immediately set to 0 (just after finishing writing the previous message to memory) as no part of a new message could fit in that area.

It is expected that input provided follows this protocol. The input variables (including ready_input) should be shared memory corresponding to an output provider's output variables.

This is a naive implementation that assumes that input and output will be produces at roughly the same speed, so no precautions are taken regarding
overwriting values in the memory areas that might not have been consumed yet.
*/

static uint64_t _enc_r_addr_begin, _enc_r_addr_end, * _enc_ready_input;
static uint64_t _enc_w_addr_begin, _enc_w_addr_end, * _enc_ready_output;

static uint64_t _dec_r_addr_begin, _dec_r_addr_end, * _dec_ready_input;
static uint64_t _dec_w_addr_begin, _dec_w_addr_end, * _dec_ready_output;

static int __encrypt();
static int __decrypt();

void crypto_main(uint64_t enc_r_addr_begin, uint64_t enc_r_addr_end, uint64_t * enc_ready_input,
                    uint64_t enc_w_addr_begin, uint64_t enc_w_addr_end, uint64_t * enc_ready_output,
                    uint64_t dec_r_addr_begin, uint64_t dec_r_addr_end, uint64_t * dec_ready_input,
                    uint64_t dec_w_addr_begin, uint64_t dec_w_addr_end, uint64_t * dec_ready_output) {

    _enc_r_addr_begin = enc_r_addr_begin; _enc_r_addr_end = enc_r_addr_end; _enc_ready_input = enc_ready_input;
    _enc_w_addr_begin = enc_w_addr_begin; _enc_w_addr_end = enc_w_addr_end; _enc_ready_output = enc_ready_output;

    _dec_r_addr_begin = dec_r_addr_begin; _dec_r_addr_end = dec_r_addr_end; _dec_ready_input = dec_ready_input;
    _dec_w_addr_begin = dec_w_addr_begin; _dec_w_addr_end = dec_w_addr_end; _dec_ready_output = dec_ready_output;

    int ret;
    while (1) {
        ret = __decrypt();
        if (ret != 1) {
            printf("FAILED TO DECRYPT\n");
        }
        ret = __encrypt();
        if (ret != 1) {
            printf("FAILED TO ENCRYPT\n");
        }
    }
}

void crypto_decrypt_main(uint64_t dec_r_addr_begin, uint64_t dec_r_addr_end, uint64_t * dec_ready_input,
                    uint64_t dec_w_addr_begin, uint64_t dec_w_addr_end, uint64_t * dec_ready_output) {

    _dec_r_addr_begin = dec_r_addr_begin; _dec_r_addr_end = dec_r_addr_end; _dec_ready_input = dec_ready_input;
    _dec_w_addr_begin = dec_w_addr_begin; _dec_w_addr_end = dec_w_addr_end; _dec_ready_output = dec_ready_output;

    int ret;
    while (1) {
        ret = __decrypt();
        switch (ret) {
        case -2:
            printf("Crypto app error: the cypher received was not a multiple of the block size %lu!\n", block_size);
            break;
        case -1: // Failed crypto initialization
        case 0: // Failed decryption
            printf("Crypto app error: failed decryption\n");
            break;
        case 1: // Success
            break;
        case 2: // Nothing to decrypt
            break;
        default:
            break;
        }
    }
}

int __encrypt() {
    printf("Error: Crypto app encryption not yet implemented\n");
    return -1;
}

static int __decrypt() {
    if (*_dec_ready_input == *_dec_ready_output) {
        // Nothing to decrypt
        return 2;
    }
    uint64_t rdy_output = *_dec_ready_output;
    // Ready output implicitly keeps track of how many bytes of input we have consumed, hence it's used to offset the begin address.
    uint64_t cypher_len = *(uint64_t *)(_dec_r_addr_begin + rdy_output);
    if (cypher_len % block_size != 0) {
        // Cypher must be multiple of block size
        return -2;
    }
    char * cypher_start = (char *)(_dec_r_addr_begin + rdy_output + message_size_byte_width);
    char * plaintext_start = (char *)(_dec_w_addr_begin + rdy_output + message_size_byte_width);
    *(uint64_t *)(_dec_w_addr_begin + rdy_output) = cypher_len;
    char cypher[cypher_len];
    char plaintext[cypher_len];
    int ret;

    if ((uint64_t)cypher_start + cypher_len > _dec_r_addr_end) {
        uint64_t first_len = (uint64_t)cypher_start + cypher_len - _dec_r_addr_end;
        for (int i = 0; i < first_len; i++) {
            cypher[i] = *(cypher_start + i);
        }
        for (int i = 0; i < cypher_len - first_len; i++) {
            cypher[i + first_len] = *(((char *)_dec_r_addr_begin) + i);
        }

        ret = decrypt_message(cypher, plaintext, cypher_len);

        for (int i = 0; i < first_len; i++) {
            *(plaintext_start + i) = plaintext[i];
        }
        for (int i = 0; i < cypher_len - first_len; i++) {
            *(((char *)_dec_w_addr_begin) + i) = plaintext[i + first_len];
        }
        // If we get here some part of the message must have been placed at the end of the region, before wrapping back to the beginning.
        // Since the message length segment is always the first part of the message, it must have been placed before wrapping back (if it wouldn't have fit before wrapping we would ahve just wrapped
        // immediately instead of splitting the message; see the end of this function), hence we don't have to consider it here.
        rdy_output = cypher_len - first_len;
    } else {
        for (int i = 0; i < cypher_len; i++) {
            cypher[i] = *(cypher_start + i);
        }
        ret = decrypt_message(cypher, plaintext, cypher_len);
        for (int i = 0; i < cypher_len; i++) {
            *(plaintext_start + i) = plaintext[i];
        }
        rdy_output += cypher_len + message_size_byte_width;
    }
    //printf("Crypto app message: %.*s\n", (int)cypher_len, plaintext);

    if ((_dec_w_addr_begin + rdy_output + message_size_byte_width) > _dec_w_addr_end) {
        // There won't be enough space left to use for the initial size segment; wrap ready tracker back to 0. 
        rdy_output = 0;
    }

    *_dec_ready_output = rdy_output;
    return ret;
}