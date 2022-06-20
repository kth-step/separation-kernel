// See LICENSE file for copyright and license details.

#include <stdio.h>
#include <stdint.h>

#include "crypto-app.h"


static uint64_t _addr_begin, _addr_end, * _ready_input;
static uint64_t consumed_input;

static int __consume();

void plaintext_consumer_main(uint64_t addr_begin, uint64_t addr_end, uint64_t * ready_input) {
    _addr_begin = addr_begin; _addr_end = addr_end; _ready_input = ready_input;
    int ret;
    while (1) {
        ret = __consume();
        if (ret != 1) {
            // Nothing to consume
        }
    }
}

static int __consume() {
    if (*_ready_input == consumed_input) {
        // Nothing to consume
        return 0;
    }
    uint64_t message_len = *(uint64_t *)(_addr_begin + consumed_input);

    char * message_start = (char *)(_addr_begin + consumed_input + message_size_byte_width);
    char message[message_len];

    if ((uint64_t)message_start + message_len > _addr_end) {
        uint64_t first_len = (uint64_t)message_start + message_len - _addr_end;
        for (int i = 0; i < first_len; i++) {
            message[i] = *(message_start + i);
        }
        for (int i = 0; i < message_len - first_len; i++) {
            message[i + first_len] = *(((char *)_addr_begin) + i);
        }

        // If we get here some part of the message must have been placed at the end of the region, before wrapping back to the beginning.
        // Since the message length segment is always the first part of the message, it must have been placed before wrapping back (if it wouldn't have fit before wrapping we would ahve just wrapped
        // immediately instead of splitting the message; see the end of this function), hence we don't have to consider it here.
        consumed_input = message_len - first_len;
    } else {
        for (int i = 0; i < message_len; i++) {
            message[i] = *(message_start + i);
        }
        consumed_input += message_len + message_size_byte_width;
    }

    if ((_addr_begin + consumed_input + message_size_byte_width) > _addr_end) {
        // There won't be enough space left to use for the initial size segment; wrap ready tracker back to 0. 
        consumed_input = 0;
    }
    printf("Message: %s\n", message);

    return 1;
}