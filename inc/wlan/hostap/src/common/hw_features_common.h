/*
 * Common hostapd/wpa_supplicant HW features
 * Copyright (c) 2002-2013, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2015, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_COMMON_HW_FEATURES_COMMON_H
#define INC_WLAN_COMMON_HW_FEATURES_COMMON_H

#include <drivers/driver.h>

struct hostapd_channel_data * hw_get_channel_chan(struct hostapd_hw_modes *mode,
						  int chan, int *freq);
struct hostapd_channel_data *
hw_mode_get_channel(struct hostapd_hw_modes *mode, int freq, int *chan);

struct hostapd_channel_data *
hw_get_channel_freq(enum hostapd_hw_mode mode, int freq, int *chan,
		    struct hostapd_hw_modes *hw_features, int num_hw_features);

int hw_get_freq(struct hostapd_hw_modes *mode, int chan);
int hw_get_chan(enum hostapd_hw_mode mode, int freq,
		struct hostapd_hw_modes *hw_features, int num_hw_features);

int allowed_ht40_channel_pair(enum hostapd_hw_mode mode,
			      struct hostapd_channel_data *p_chan,
			      struct hostapd_channel_data *s_chan);
void get_pri_sec_chan(struct wpa_scan_res *bss, int *pri_chan, int *sec_chan);
int check_40mhz_5g(struct wpa_scan_results *scan_res,
		   struct hostapd_channel_data *pri_chan,
		   struct hostapd_channel_data *sec_chan);
int check_40mhz_2g4(struct hostapd_hw_modes *mode,
		    struct wpa_scan_results *scan_res, int pri_chan,
		    int sec_chan);
void punct_update_legacy_bw(uint16_t bitmap, uint8_t pri_chan,
			    enum oper_chan_width *width, uint8_t *seg0, uint8_t *seg1);
int hostapd_set_freq_params(struct hostapd_freq_params *data,
			    enum hostapd_hw_mode mode,
			    int freq, int channel, int edmg, uint8_t edmg_channel,
			    int ht_enabled,
			    int vht_enabled, int he_enabled,
			    bool eht_enabled, int sec_channel_offset,
			    enum oper_chan_width oper_chwidth,
			    int center_segment0,
			    int center_segment1, uint32_t vht_caps,
			    struct he_capabilities *he_caps,
			    struct eht_capabilities *eht_cap,
			    uint16_t punct_bitmap);
void set_disable_ht40(struct ieee80211_ht_capabilities *htcaps,
		      int disabled);
int ieee80211ac_cap_check(uint32_t hw, uint32_t conf);

uint32_t num_chan_to_bw(int num_chans);
int chan_bw_allowed(const struct hostapd_channel_data *chan, uint32_t bw,
		    int ht40_plus, int pri);
int chan_pri_allowed(const struct hostapd_channel_data *chan);
bool is_punct_bitmap_valid(uint16_t bw, uint16_t pri_ch_bit_pos, uint16_t punct_bitmap);

#endif /* HW_FEATURES_COMMON_H */
