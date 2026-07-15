/*
 * AES functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_AES_H
#define INC_WLAN_CRYPTO_AES_H

#include <types.h>

#define AES_BLOCK_SIZE 16

void * aes_encrypt_init(const uint8_t *key, size_t len);
int aes_encrypt(void *ctx, const uint8_t *plain, uint8_t *crypt);
void aes_encrypt_deinit(void *ctx);
void * aes_decrypt_init(const uint8_t *key, size_t len);
int aes_decrypt(void *ctx, const uint8_t *crypt, uint8_t *plain);
void aes_decrypt_deinit(void *ctx);

#endif /* AES_H */
