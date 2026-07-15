/*
 * wpa_supplicant - WPA/RSN IE and KDE definitions
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_RSN_SUPP_WPA_IE_H
#define INC_WLAN_RSN_SUPP_WPA_IE_H

#include <types.h>

struct wpa_sm;

int wpa_gen_wpa_ie(struct wpa_sm *sm, uint8_t *wpa_ie, size_t wpa_ie_len);
int wpa_gen_rsnxe(struct wpa_sm *sm, uint8_t *rsnxe, size_t rsnxe_len);
uint16_t rsn_supp_capab(struct wpa_sm *sm);

#endif /* WPA_IE_H */
