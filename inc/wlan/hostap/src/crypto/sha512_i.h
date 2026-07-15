/*
 * SHA-512 internal definitions
 * Copyright (c) 2015, Pali Rohár <pali.rohar@gmail.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_SHA512_I_H
#define INC_WLAN_CRYPTO_SHA512_I_H

#include <types.h>

#define SHA512_BLOCK_SIZE 128

struct sha512_state {
	uint64_t length, state[8];
	uint32_t curlen;
	uint8_t buf[SHA512_BLOCK_SIZE];
};

void sha512_init(struct sha512_state *md);
int sha512_process(struct sha512_state *md, const unsigned char *in,
		   unsigned long inlen);
int sha512_done(struct sha512_state *md, unsigned char *out);

#endif /* SHA512_I_H */
