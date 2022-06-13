#include <stdio.h>

#include "wolfssl/wolfssl/wolfcrypt/aes.h"
#include "crypto.h"

void test_wolfssl();
void test_crypto_app();

int main() {
    test_crypto_app();
    return 0;
}

void test_crypto_app() {
    unsigned char cypher[16] = "";
    unsigned char str[] = {(char)0xF2,(char)0x95,(char)0xB9,(char)0x31,
                    (char)0x8B,(char)0x99,(char)0x44,(char)0x34,
                    (char)0xD9,(char)0x3D,(char)0x98,(char)0xA4,
                    (char)0xE4,(char)0x49,(char)0xAF,(char)0xD8}; //"f295b9318b994434d93d98a4e449afd8"
    encrypt_message(str, cypher, 16);
    unsigned char str1[16];
    decrypt_message(cypher, str1, 16);
    printf("str:    ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", str[i]);
    }
    printf("\n");
    printf("Cypher: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", cypher[i]);
    }
    printf("\n");
    printf("Str1:   ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", str1[i]);
    }
    printf("\n\n");

    char str2[] = "hej123456"; // 9
    char cypher2[16] = "";
    char str3[16];

    int ret = encrypt_message(str2, cypher2, 16);
    printf("ret: %d, cyp2: %s\n", ret, cypher2);

    ret = decrypt_message(cypher2, str3, 16);
    printf("ret: %d, str3: %s\n\n", ret, str3);

    char str4[16];
    for (int i = 0; i < 16; i++) {
        str4[i] = '\0';
    }
    ret = encrypt_message(str2, cypher2, 8);
    printf("ret: %d, cyp2: %s\n", ret, cypher2);

    ret = decrypt_message(cypher2, str4, 4);
    printf("ret: %d, str4: %s\n", ret, str4);
}

void test_wolfssl() {
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
    wc_AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_DECRYPTION);
        if (wc_AesCbcDecrypt(&enc, input, actual_output, len) == 0) {
            printf("\nSuccess decrypting message.\nResult:    ");
            for (int i = 0; i < len; i++) {
                printf("%02x", input[i]);
            }
            printf("\n");
        }
    wc_AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_ENCRYPTION);
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
        wc_AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_DECRYPTION);
        if (wc_AesCbcDecrypt(&enc, input, actual_output, len) == 0) {
            printf("\nSuccess decrypting message.\nResult:    ");
            for (int i = 0; i < len; i++) {
                printf("%02x", input[i]);
            }
            printf("\n");
        }
    }
}