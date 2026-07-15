/*
 * DES and 3DES-EDE ciphers
 * Copyright (c) 2006-2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_DES_I_H
#define INC_WLAN_CRYPTO_DES_I_H

#include <types.h>

struct des3_key_s {
	uint32_t ek[3][32];
	uint32_t dk[3][32];
};

void des_key_setup(const uint8_t *key, uint32_t *ek, uint32_t *dk);
void des_block_encrypt(const uint8_t *plain, const uint32_t *ek, uint8_t *crypt);
void des_block_decrypt(const uint8_t *crypt, const uint32_t *dk, uint8_t *plain);

void des3_key_setup(const uint8_t *key, struct des3_key_s *dkey);
void des3_encrypt(const uint8_t *plain, const struct des3_key_s *key, uint8_t *crypt);
void des3_decrypt(const uint8_t *crypt, const struct des3_key_s *key, uint8_t *plain);

#endif /* DES_I_H */
