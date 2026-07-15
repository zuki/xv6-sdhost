/*
 * EAP-IKEv2 definitions
 * Copyright (c) 2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_IKEV2_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_IKEV2_COMMON_H

#include <types.h>

#define IKEV2_FLAGS_LENGTH_INCLUDED 0x80
#define IKEV2_FLAGS_MORE_FRAGMENTS 0x40
#define IKEV2_FLAGS_ICV_INCLUDED 0x20

#define IKEV2_FRAGMENT_SIZE 1400

struct ikev2_keys;

int eap_ikev2_derive_keymat(int prf, struct ikev2_keys *keys,
			    const uint8_t *i_nonce, size_t i_nonce_len,
			    const uint8_t *r_nonce, size_t r_nonce_len,
			    uint8_t *keymat);
struct wpabuf * eap_ikev2_build_frag_ack(uint8_t id, uint8_t code);
int eap_ikev2_validate_icv(int integ_alg, struct ikev2_keys *keys,
			   int initiator, const struct wpabuf *msg,
			   const uint8_t *pos, const uint8_t *end);

#endif /* EAP_IKEV2_COMMON_H */
