/*
 * MSCHAPV2 (RFC 2759)
 * Copyright (c) 2004-2008, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_PEER_MSCHAPV2_H
#define INC_WLAN_EAP_PEER_MSCHAPV2_H

#include <types.h>

#define MSCHAPV2_CHAL_LEN 16
#define MSCHAPV2_NT_RESPONSE_LEN 24
#define MSCHAPV2_AUTH_RESPONSE_LEN 20
#define MSCHAPV2_MASTER_KEY_LEN 16

const uint8_t * mschapv2_remove_domain(const uint8_t *username, size_t *len);
int mschapv2_derive_response(const uint8_t *username, size_t username_len,
			     const uint8_t *password, size_t password_len,
			     int pwhash,
			     const uint8_t *auth_challenge,
			     const uint8_t *peer_challenge,
			     uint8_t *nt_response, uint8_t *auth_response,
			     uint8_t *master_key);
int mschapv2_verify_auth_response(const uint8_t *auth_response,
				  const uint8_t *buf, size_t buf_len);

#endif /* MSCHAPV2_H */
