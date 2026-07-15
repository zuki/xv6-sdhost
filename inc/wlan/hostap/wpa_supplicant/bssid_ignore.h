/*
 * wpa_supplicant - List of temporarily ignored BSSIDs
 * Copyright (c) 2003-2021, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WPA_SUPPLICANT_BSSID_IGNORE_H
#define INC_WPA_SUPPLICANT_BSSID_IGNORE_H

#include <types.h>

struct wpa_bssid_ignore {
	struct wpa_bssid_ignore *next;
	uint8_t bssid[ETH_ALEN];
	int count;
	/* Time of the most recent trigger to ignore this BSSID. */
	struct os_reltime start;
	/*
	 * Number of seconds after start that the entey will be considered
	 * valid.
	 */
	int timeout_secs;
};

struct wpa_bssid_ignore * wpa_bssid_ignore_get(struct wpa_supplicant *wpa_s,
					 const uint8_t *bssid);
int wpa_bssid_ignore_add(struct wpa_supplicant *wpa_s, const uint8_t *bssid);
int wpa_bssid_ignore_del(struct wpa_supplicant *wpa_s, const uint8_t *bssid);
int wpa_bssid_ignore_is_listed(struct wpa_supplicant *wpa_s, const uint8_t *bssid);
void wpa_bssid_ignore_clear(struct wpa_supplicant *wpa_s);
void wpa_bssid_ignore_update(struct wpa_supplicant *wpa_s);

#endif /* BSSID_IGNORE_H */
