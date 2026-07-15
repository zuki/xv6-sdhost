/*
 * SHA384 hash implementation and interface functions
 * Copyright (c) 2015-2017, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_SHA384_H
#define INC_WLAN_CRYPTO_SHA384_H

#include <types.h>

#define SHA384_MAC_LEN 48

int hmac_sha384_vector(const uint8_t *key, size_t key_len, size_t num_elem,
		       const uint8_t *addr[], const size_t *len, uint8_t *mac);
int hmac_sha384(const uint8_t *key, size_t key_len, const uint8_t *data,
		size_t data_len, uint8_t *mac);
int sha384_prf(const uint8_t *key, size_t key_len, const char *label,
	       const uint8_t *data, size_t data_len, uint8_t *buf, size_t buf_len);
int sha384_prf_bits(const uint8_t *key, size_t key_len, const char *label,
		    const uint8_t *data, size_t data_len, uint8_t *buf,
		    size_t buf_len_bits);
int tls_prf_sha384(const uint8_t *secret, size_t secret_len,
		   const char *label, const uint8_t *seed, size_t seed_len,
		   uint8_t *out, size_t outlen);
int hmac_sha384_kdf(const uint8_t *secret, size_t secret_len,
		    const char *label, const uint8_t *seed, size_t seed_len,
		    uint8_t *out, size_t outlen);

#endif /* SHA384_H */
