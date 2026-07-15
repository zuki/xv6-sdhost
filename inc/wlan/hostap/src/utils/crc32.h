/*
 * 32-bit CRC for FCS calculation
 * Copyright (c) 2010, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_UTILS_CRC32_H
#define INC_WLAN_UTILS_CRC32_H

#include <types.h>

uint32_t ieee80211_crc32(const uint8_t *frame, size_t frame_len);

#endif /* CRC32_H */
