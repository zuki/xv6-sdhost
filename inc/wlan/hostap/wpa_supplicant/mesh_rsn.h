/*
 * WPA Supplicant - Mesh RSN routines
 * Copyright (c) 2013-2014, cozybit, Inc.  All rights reserved.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WPA_SUPPLICANT_MESH_RSN_H
#define INC_WPA_SUPPLICANT_MESH_RSN_H

#include <types.h>

struct mesh_rsn {
	struct wpa_supplicant *wpa_s;
	struct wpa_authenticator *auth;
	unsigned int pairwise_cipher;
	unsigned int group_cipher;
	uint8_t mgtk[WPA_TK_MAX_LEN];
	size_t mgtk_len;
	uint8_t mgtk_key_id;
	unsigned int mgmt_group_cipher;
	uint8_t igtk_key_id;
	uint8_t igtk[WPA_TK_MAX_LEN];
	size_t igtk_len;
#ifdef CONFIG_SAE
	struct wpabuf *sae_token;
	int sae_group_index;
#endif /* CONFIG_SAE */
};

struct mesh_rsn * mesh_rsn_auth_init(struct wpa_supplicant *wpa_s,
				     struct mesh_conf *conf);
int mesh_rsn_auth_sae_sta(struct wpa_supplicant *wpa_s, struct sta_info *sta);
int mesh_rsn_derive_mtk(struct wpa_supplicant *wpa_s, struct sta_info *sta);
void mesh_rsn_get_pmkid(struct mesh_rsn *rsn, struct sta_info *sta, uint8_t *pmkid);
void mesh_rsn_init_ampe_sta(struct wpa_supplicant *wpa_s,
			    struct sta_info *sta);
int mesh_rsn_protect_frame(struct mesh_rsn *rsn, struct sta_info *sta,
			   const uint8_t *cat, struct wpabuf *buf);
int mesh_rsn_process_ampe(struct wpa_supplicant *wpa_s, struct sta_info *sta,
			  struct ieee802_11_elems *elems, const uint8_t *cat,
			  const uint8_t *chosen_pmk,
			  const uint8_t *start, size_t elems_len);
void mesh_auth_timer(void *eloop_ctx, void *user_data);

#endif /* MESH_RSN_H */
