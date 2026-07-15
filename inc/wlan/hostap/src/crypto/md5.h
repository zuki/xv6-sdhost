/*
 * MD5 hash implementation and interface functions
 * Copyright (c) 2003-2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_MD5_H
#define INC_WLAN_CRYPTO_MD5_H

#include <types.h>

#define MD5_MAC_LEN 16

int hmac_md5_vector(const uint8_t *key, size_t key_len, size_t num_elem,
		    const uint8_t *addr[], const size_t *len, uint8_t *mac);
int hmac_md5(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len,
	     uint8_t *mac);

#endif /* MD5_H */
