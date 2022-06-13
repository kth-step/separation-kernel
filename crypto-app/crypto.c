// See LICENSE file for copyright and license details.
#include "crypto.h"

static Aes enc;
static Aes dec;
static byte key[] = {   0xF4,0xC0,0x20,0xA0,
                        0xA1,0xF6,0x04,0xFD,
                        0x34,0x3F,0xAC,0x6A,
                        0x7E,0x6A,0xE0,0xF9};
static byte iv[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int initilaized = 0;

/* Initialize the encryption and decription structs. Returns 1 if successful, otherwise 0. */
static int __init() {
    // Set key returns 0 on success
    int ret1 = wc_AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_ENCRYPTION);
    int ret2 = wc_AesSetKey(&dec, key, AES_BLOCK_SIZE, iv, AES_DECRYPTION);
    return (ret1 == 0) && (ret2 == 0);
}

/*  message is the text to encrypt. It must be a multiple of 16 bytes.
    cypher is the array where the encrypted text will be placed. This must be a multiple of 16 with enough space for the message.
    num_blocks is the number of 16 byte blocks the message to be encrypted contains.
    Returns 1 on success, 0 on failed encryption, and -1 on failed initialization of encryption struct. */
static int __encrypt(const byte message[], byte cypher[], int num_blocks) {
    int ret = 0;
    if (!initilaized) {
        ret = __init();
        if (!ret)
            return -1;
        initilaized = 1;
    }
    // Encrypt returns 0 on success
    ret = wc_AesCbcEncrypt(&enc, cypher, message, AES_BLOCK_SIZE * num_blocks);
    return ret == 0;
}

/*  cypher is the text to decrypt. It must be a multiple of 16 bytes.
    plaintext is the array where the decrypted text will be placed.
    num_blocks is the number of 16 byte blocks the message to be decrypted contains.
    Returns 1 on success, 0 on failed decryption, and -1 on failed initialization of decryption struct. */
static int __decrypt(const byte cypher[], byte plaintext[], int num_blocks) {
    int ret = 0;
    if (!initilaized) {
        ret = __init();
        if (!ret)
            return -1;
        initilaized = 1;
    }
    // Decrypt returns 0 on success
    ret = wc_AesCbcDecrypt(&dec, plaintext, cypher, AES_BLOCK_SIZE * num_blocks);
    return ret == 0;
}

/*  message is the text to encrypt. 
    cypher is the array where the encrypted text will be placed. The size must be at least the smallest multiple of 16 greater than or equal to the message_len argument.
    message_len is the number of bytes to encrypt from the message argument. If it is not a multiple of 16 the message will be padded with null bytes up to the nearest multiple.
    Returns 1 on success, 0 on failed encryption, and -1 on failed initialization of encryption struct. */
int encrypt_message(const byte message[], byte cypher[], int message_len) {
    int aligned_len = message_len;
    if (message_len % AES_BLOCK_SIZE != 0) {
        aligned_len = ((message_len / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
    }
    byte m[aligned_len];
    for (int i = 0; i < message_len; i++) {
        m[i] = message[i];
    }
    for (int i = message_len; i < aligned_len; i++) {
        m[i] = (byte)'\0';
    }

    return __encrypt(m, cypher, aligned_len / AES_BLOCK_SIZE);
    
}

/*  cypher is the text to decrypt. It must be a multiple of 16 bytes.
    plaintext is the array where the decrypted text will be placed. It must be able to contain at least @arg message_len number of bytes.
    message_len is the number of bytes to return from the decrypted text. This can be equal to the size of the cypher, or less if one expects the cypher to have encrypted a number of padding bytes. 
    Note that the nearest multiple of 16 bytes equal to or greater than message_len will be taken as the size of the cypher.
    Returns 1 on success, 0 on failed decryption, -1 on failed initialization of decryption struct. */
int decrypt_message(const byte cypher[], byte plaintext[], int message_len) {
    int aligned_len = message_len;
    if (message_len % AES_BLOCK_SIZE != 0) {
        aligned_len = ((message_len / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
    }
    byte p[aligned_len];
    int ret = __decrypt(cypher, p, aligned_len / AES_BLOCK_SIZE);
    for (int i = 0; i < message_len; i++) {
        plaintext[i] = p[i];
    }
    return ret; 
}