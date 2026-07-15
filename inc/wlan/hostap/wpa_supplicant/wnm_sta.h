/*
 * IEEE 802.11v WNM related functions and structures
 * Copyright (c) 2011-2012, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WPA_SUPPLICANT_WNM_STA_H
#define INC_WPA_SUPPLICANT_WNM_STA_H

#include <types.h>

struct measurement_pilot {
	uint8_t measurement_pilot;
	uint8_t subelem_len;
	uint8_t subelems[255];
};

struct multiple_bssid {
	uint8_t max_bssid_indicator;
	uint8_t subelem_len;
	uint8_t subelems[255];
};

struct neighbor_report {
	uint8_t bssid[ETH_ALEN];
	uint32_t bssid_info;
	uint8_t regulatory_class;
	uint8_t channel_number;
	uint8_t phy_type;
	uint8_t preference; /* valid if preference_present=1 */
	uint16_t tsf_offset; /* valid if tsf_present=1 */
	uint16_t beacon_int; /* valid if tsf_present=1 */
	char country[2]; /* valid if country_present=1 */
	uint8_t rm_capab[5]; /* valid if rm_capab_present=1 */
	uint16_t bearing; /* valid if bearing_present=1 */
	uint16_t rel_height; /* valid if bearing_present=1 */
	uint32_t distance; /* valid if bearing_present=1 */
	uint64_t bss_term_tsf; /* valid if bss_term_present=1 */
	uint16_t bss_term_dur; /* valid if bss_term_present=1 */
	unsigned int disassoc_imminent:1;
	unsigned int preference_present:1;
	unsigned int tsf_present:1;
	unsigned int country_present:1;
	unsigned int rm_capab_present:1;
	unsigned int bearing_present:1;
	unsigned int bss_term_present:1;
	unsigned int acceptable:1;
#ifdef CONFIG_MBO
	unsigned int is_first:1;
#endif /* CONFIG_MBO */
	struct measurement_pilot *meas_pilot;
	struct multiple_bssid *mul_bssid;
	int freq;
};


int ieee802_11_send_wnmsleep_req(struct wpa_supplicant *wpa_s,
				 uint8_t action, uint16_t intval, struct wpabuf *tfs_req);

void ieee802_11_rx_wnm_action(struct wpa_supplicant *wpa_s,
			      const struct ieee80211_mgmt *mgmt, size_t len);

int wnm_send_bss_transition_mgmt_query(struct wpa_supplicant *wpa_s,
				       uint8_t query_reason,
				       const char *btm_candidates,
				       int cand_list);

int wnm_send_coloc_intf_report(struct wpa_supplicant *wpa_s, uint8_t dialog_token,
			       const struct wpabuf *elems);
void wnm_set_coloc_intf_elems(struct wpa_supplicant *wpa_s,
			      struct wpabuf *elems);

int wnm_btm_resp_tx_status(struct wpa_supplicant *wpa_s, const uint8_t *data,
			   size_t data_len);

#ifdef CONFIG_WNM

int wnm_scan_process(struct wpa_supplicant *wpa_s, bool pre_scan_check);
void wnm_clear_coloc_intf_reporting(struct wpa_supplicant *wpa_s);

bool wnm_is_bss_excluded(struct wpa_supplicant *wpa_s, struct wpa_bss *bss);

void wnm_btm_reset(struct wpa_supplicant *wpa_s);

static inline bool wnm_active_bss_trans_mgmt(struct wpa_supplicant *wpa_s)
{
	return !!wpa_s->wnm_dialog_token;
}

#else /* CONFIG_WNM */

static inline int wnm_scan_process(struct wpa_supplicant *wpa_s,
				   int reply_on_fail)
{
	return 0;
}

static inline void wnm_clear_coloc_intf_reporting(struct wpa_supplicant *wpa_s)
{
}

static inline bool
wnm_is_bss_excluded(struct wpa_supplicant *wpa_s, struct wpa_bss *bss)
{
	return false;
}

static inline void wnm_btm_reset(struct wpa_supplicant *wpa_s)
{
}

static inline bool wnm_active_bss_trans_mgmt(struct wpa_supplicant *wpa_s)
{
	return false;
}

#endif /* CONFIG_WNM */

#endif /* WNM_STA_H */
