/*
 * Interworking (IEEE 802.11u)
 * Copyright (c) 2011-2012, Qualcomm Atheros
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WPA_SUPPLICANT_INTERWORKING_H
#define INC_WPA_SUPPLICANT_INTERWORKING_H

#include <types.h>

enum gas_query_result;

int anqp_send_req(struct wpa_supplicant *wpa_s, const uint8_t *dst, int freq,
		  uint16_t info_ids[], size_t num_ids, uint32_t subtypes,
		  uint32_t mbo_subtypes);
void anqp_resp_cb(void *ctx, const uint8_t *dst, uint8_t dialog_token,
		  enum gas_query_result result,
		  const struct wpabuf *adv_proto,
		  const struct wpabuf *resp, uint16_t status_code);
int gas_send_request(struct wpa_supplicant *wpa_s, const uint8_t *dst,
		     const struct wpabuf *adv_proto,
		     const struct wpabuf *query);
int interworking_fetch_anqp(struct wpa_supplicant *wpa_s);
void interworking_stop_fetch_anqp(struct wpa_supplicant *wpa_s);
int interworking_select(struct wpa_supplicant *wpa_s, int auto_select,
			int *freqs);
int interworking_connect(struct wpa_supplicant *wpa_s, struct wpa_bss *bss,
			 int only_add);
void interworking_start_fetch_anqp(struct wpa_supplicant *wpa_s);
int interworking_home_sp_cred(struct wpa_supplicant *wpa_s,
			      struct wpa_cred *cred,
			      struct wpabuf *domain_names);
int domain_name_list_contains(struct wpabuf *domain_names,
			      const char *domain, int exact_match);

#endif /* INTERWORKING_H */
