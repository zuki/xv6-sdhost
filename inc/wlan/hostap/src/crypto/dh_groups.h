/*
 * Diffie-Hellman groups
 * Copyright (c) 2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_DH_GROUPS_H
#define INC_WLAN_CRYPTO_DH_GROUPS_H

#include <types.h>

struct dh_group {
	int id;
	const uint8_t *generator;
	size_t generator_len;
	const uint8_t *prime;
	size_t prime_len;
	const uint8_t *order;
	size_t order_len;
	unsigned int safe_prime:1;
};

const struct dh_group * dh_groups_get(int id);
struct wpabuf * dh_init(const struct dh_group *dh, struct wpabuf **priv);
struct wpabuf * dh_derive_shared(const struct wpabuf *peer_public,
				 const struct wpabuf *own_private,
				 const struct dh_group *dh);

#endif /* DH_GROUPS_H */
