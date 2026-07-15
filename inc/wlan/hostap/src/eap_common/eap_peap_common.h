/*
 * EAP-PEAP common routines
 * Copyright (c) 2008-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_PEAP_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_PEAP_COMMON_H

#include <types.h>

int peap_prfplus(int version, const uint8_t *key, size_t key_len,
		 const char *label, const uint8_t *seed, size_t seed_len,
		 uint8_t *buf, size_t buf_len);

#endif /* EAP_PEAP_COMMON_H */
