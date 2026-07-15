/*
 * AES SIV (RFC 5297)
 * Copyright (c) 2013 Cozybit, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_AES_SIV_H
#define INC_WLAN_CRYPTO_AES_SIV_H

#include <types.h>

int aes_siv_encrypt(const uint8_t *key, size_t key_len,
		    const uint8_t *pw, size_t pwlen,
		    size_t num_elem, const uint8_t *addr[], const size_t *len,
		    uint8_t *out);
int aes_siv_decrypt(const uint8_t *key, size_t key_len,
		    const uint8_t *iv_crypt, size_t iv_c_len,
		    size_t num_elem, const uint8_t *addr[], const size_t *len,
		    uint8_t *out);

#endif /* AES_SIV_H */
