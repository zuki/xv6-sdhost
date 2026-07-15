/*
 * CHAP-MD5 (RFC 1994)
 * Copyright (c) 2007-2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_CHAP_H
#define INC_WLAN_EAP_COMMON_CHAP_H

#include <types.h>

#define CHAP_MD5_LEN 16

int chap_md5(uint8_t id, const uint8_t *secret, size_t secret_len, const uint8_t *challenge,
	     size_t challenge_len, uint8_t *response);

#endif /* CHAP_H */
