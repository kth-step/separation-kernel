#include <stdio.h>

#include "wolfssl/wolfssl/wolfcrypt/aes.h"

int main() {
    // For relevant docs: /wolfssl-master/doc/dox_comments/header_files/aes.h
    Aes enc;
    int len = 16;
    int ret = 0;
    byte key[] = {  0xF4,0xC0,0x20,0xA0,
                    0xA1,0xF6,0x04,0xFD,
                    0x34,0x3F,0xAC,0x6A,
                    0x7E,0x6A,0xE0,0xF9};//{ some 16, 24 or 32 byte key };
    byte iv[]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//{ some 16 byte iv };
    if (ret = wc_AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_ENCRYPTION) != 0) {
	    printf("Failed to set AES key\n");
    } else printf("Success setting key\n");

    byte input[] = {0xF2,0x95,0xB9,0x31,
                    0x8B,0x99,0x44,0x34,
                    0xD9,0x3D,0x98,0xA4,
                    0xE4,0x49,0xAF,0xD8};

    byte expected_output[] = {  0x52,0xE4,0x18,0xCB,
                                0xB1,0xBE,0x49,0x49,
                                0x30,0x8B,0x38,0x16,
                                0x91,0xB1,0x09,0xFE};

    byte actual_output[len];

    if (wc_AesCbcEncrypt(&enc, actual_output, input, len) != 0) {
        printf("Failed to encrypt message!\n");
    } else {
        printf("Success encrypting message.\nInput:    ");
        for (int i = 0; i < len; i++) {
            printf("%02x", input[i]);
        }
        printf("\nActual:   ");
        for (int i = 0; i < len; i++) {
            printf("%02x", actual_output[i]);
        }
        printf("\nExpected: ");
        for (int i = 0; i < len; i++) {
            printf("%02x", expected_output[i]);
        }
        printf("\n");
        int matching = 1;
        for (int i = 0; i < len; i++) {
            if (actual_output[i] != expected_output[i]) {
                matching = 0;
                break;
            }
        }
        if (matching) {
            printf("Actual matches expected\n");
        } else {
            printf("Actual does NOT match expected\n");
        }
    }
    printf("\n\n");
    if (wc_AesCbcEncrypt(&enc, actual_output, input, len) != 0) {
        printf("Failed to encrypt message!\n");
    } else {
        printf("Success encrypting message.\nInput:    ");
        for (int i = 0; i < len; i++) {
            printf("%02x", input[i]);
        }
        printf("\nActual:   ");
        for (int i = 0; i < len; i++) {
            printf("%02x", actual_output[i]);
        }
        printf("\nExpected: ");
        for (int i = 0; i < len; i++) {
            printf("%02x", expected_output[i]);
        }
        printf("\n");
        int matching = 1;
        for (int i = 0; i < len; i++) {
            if (actual_output[i] != expected_output[i]) {
                matching = 0;
                break;
            }
        }
        if (matching) {
            printf("Actual matches expected\n");
        } else {
            printf("Actual does NOT match expected\n");
        }
    }

    printf("\n\n");
    if (wc_AesCbcEncrypt(&enc, actual_output, input, len) != 0) {
        printf("Failed to encrypt message!\n");
    } else {
        printf("Success encrypting message.\nInput:    ");
        for (int i = 0; i < len; i++) {
            printf("%02x", input[i]);
        }
        printf("\nActual:   ");
        for (int i = 0; i < len; i++) {
            printf("%02x", actual_output[i]);
        }
        printf("\nExpected: ");
        for (int i = 0; i < len; i++) {
            printf("%02x", expected_output[i]);
        }
        printf("\n");
        int matching = 1;
        for (int i = 0; i < len; i++) {
            if (actual_output[i] != expected_output[i]) {
                matching = 0;
                break;
            }
        }
        if (matching) {
            printf("Actual matches expected\n");
        } else {
            printf("Actual does NOT match expected\n");
        }
    }

    return 0;
}