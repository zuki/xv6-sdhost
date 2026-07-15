/*
 * Generic advertisement service (GAS) query
 * Copyright (c) 2009, Atheros Communications
 * Copyright (c) 2011, Qualcomm Atheros
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WPA_SUPPLICANT_GAS_QUERY_H
#define INC_WPA_SUPPLICANT_GAS_QUERY_H

struct gas_query;

#ifdef CONFIG_GAS

struct gas_query * gas_query_init(struct wpa_supplicant *wpa_s);
void gas_query_deinit(struct gas_query *gas);
int gas_query_rx(struct gas_query *gas, const uint8_t *da, const uint8_t *sa,
		 const uint8_t *bssid, uint8_t categ, const uint8_t *data, size_t len,
		 int freq);

/**
 * enum gas_query_result - GAS query result
 */
enum gas_query_result {
	GAS_QUERY_SUCCESS,
	GAS_QUERY_FAILURE,
	GAS_QUERY_TIMEOUT,
	GAS_QUERY_PEER_ERROR,
	GAS_QUERY_INTERNAL_ERROR,
	GAS_QUERY_STOPPED,
	GAS_QUERY_DELETED_AT_DEINIT
};

int gas_query_req(struct gas_query *gas, const uint8_t *dst, int freq,
		  int wildcard_bssid, int maintain_addr, struct wpabuf *req,
		  void (*cb)(void *ctx, const uint8_t *dst, uint8_t dialog_token,
			     enum gas_query_result result,
			     const struct wpabuf *adv_proto,
			     const struct wpabuf *resp, uint16_t status_code),
		  void *ctx);
int gas_query_stop(struct gas_query *gas, uint8_t dialog_token);

#else /* CONFIG_GAS */

static inline struct gas_query * gas_query_init(struct wpa_supplicant *wpa_s)
{
	return (void *) 1;
}

static inline void gas_query_deinit(struct gas_query *gas)
{
}

#endif /* CONFIG_GAS */


#endif /* GAS_QUERY_H */
