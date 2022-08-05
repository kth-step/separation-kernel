// See LICENSE file for copyright and license details.

#include <stdio.h>

#include "crypto.h"
#include "crypto-app.h"
#include "../src/timer.h"

const uint64_t plaintext_len = 160;
char plaintext[] =  "A separation kernel is a type of"
                    " security kernel used to simulat"
                    "e a distributed environment. The"
                    " concept was introduced by John "
                    "Rushby in a 1981 paper.\n12345\n\n";
const uint64_t message_len = 32;
static uint64_t _addr_begin, _addr_end, * _ready_output;

int cypher_counter = 0;
uint64_t begin_time;

static int __provide();

void cypher_provider_main(uint64_t addr_begin, uint64_t addr_end, uint64_t * ready_output) {
    begin_time = read_time();
    _addr_begin = addr_begin; _addr_end = addr_end; _ready_output = ready_output;
    if (sizeof(plaintext) != plaintext_len) {
        printf("ERROR PLAINTEXT IS OF INCORRECT SIZE: ");
        printf("Actual: %lu vs expected: %lu\n", sizeof(plaintext), plaintext_len);
        return;
    }
    int ret;
    while (1) {
        ret = __provide();
        switch (ret) {
        case -1: // Failed crypto initialization
        case 0: // Failed encryption
            printf("Cypher provider failed encryption\n");
            break;
        case 1: // Success
            break;
        default:
            break;
        }
    }
}

static int __provide() {
    uint64_t rdy_output = *_ready_output;
    char * cypher_start = (char *)(_addr_begin + rdy_output + message_size_byte_width);
    *(uint64_t *)(_addr_begin + rdy_output) = message_len;
    char cypher[message_len];
    int ret = encrypt_message(&plaintext[(cypher_counter * message_len) % plaintext_len], cypher, message_len);

    if ((uint64_t)cypher_start + message_len > _addr_end) {
        /* Stop execution when we reach address end. 
           While we have a solution to wrap around, currently the producer produce too fast meaning it overwrites
           values the consumer has not yet consumed, and this creates issues. */
        while(1);

        uint64_t first_len = (uint64_t)cypher_start + message_len - _addr_end;
        for (int i = 0; i < first_len; i++) {
            *(cypher_start + i) = cypher[i];
        }
        for (int i = 0; i < message_len - first_len; i++) {
            *(((char *)_addr_begin) + i) = cypher[i + first_len];
        }
        // If we get here some part of the message must have been placed at the end of the region, before wrapping back to the beginning.
        // Since the message length segment is always the first part of the message, it must have been placed before wrapping back 
        // (if it wouldn't have fit before wrapping we would have just wrapped immediately instead of splitting the message; see the end of this function), 
        // hence we don't have to consider it here.
        rdy_output = message_len - first_len;
    } else {
        for (int i = 0; i < message_len; i++) {
            *(cypher_start + i) = cypher[i];
        }
        rdy_output += message_len + message_size_byte_width;
    }

    if ((_addr_begin + rdy_output + message_size_byte_width) > _addr_end) {
        // There won't be enough space left to use for the initial size segment; wrap ready tracker back to 0. 
        rdy_output = 0;
    }
    cypher_counter += 1;
    if (cypher_counter == plaintext_len)
        cypher_counter = 0;

    *_ready_output = rdy_output;
    return ret;
}