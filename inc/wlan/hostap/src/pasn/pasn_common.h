/*
 * PASN info for initiator and responder
 *
 * Copyright (C) 2019, Intel Corporation
 * Copyright (c) 2022, Jouni Malinen <j@w1.fi>
 * Copyright (C) 2022, Qualcomm Innovation Center, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_PASN_COMMON_H
#define INC_WLAN_PASN_COMMON_H

#include <types.h>
#include <common/ieee802_11_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

enum pasn_fils_state {
	PASN_FILS_STATE_NONE = 0,
	PASN_FILS_STATE_PENDING_AS,
	PASN_FILS_STATE_COMPLETE
};

struct pasn_fils {
	uint8_t state;
	uint8_t nonce[FILS_NONCE_LEN];
	uint8_t anonce[FILS_NONCE_LEN];
	uint8_t session[FILS_SESSION_LEN];
	uint8_t erp_pmkid[PMKID_LEN];
	bool completed;
	struct wpabuf *erp_resp;
};

struct pasn_data {
	/* External modules access below variables using setter and getter
	 * functions */
	int akmp;
	int cipher;
	uint8_t own_addr[ETH_ALEN];
	uint8_t peer_addr[ETH_ALEN];
	uint8_t bssid[ETH_ALEN];
	struct rsn_pmksa_cache *pmksa;
	bool derive_kdk;
	size_t kdk_len;
	void *cb_ctx;

#ifdef CONFIG_SAE
	struct sae_pt *pt;
#endif /* CONFIG_SAE */

	/* Responder */
	const char *password;
	int wpa_key_mgmt;
	int rsn_pairwise;
	uint16_t rsnxe_capab;
	const uint8_t *rsnxe_ie;
	bool custom_pmkid_valid;
	uint8_t custom_pmkid[PMKID_LEN];

	/*
	 * Extra elements to add into Authentication frames. These can be used,
	 * e.g., for Wi-Fi Aware use cases.
	 */
	const uint8_t *extra_ies;
	size_t extra_ies_len;

	/* External modules do not access below variables */
	uint16_t group;
	bool secure_ltf;
	int freq;

	uint8_t trans_seq;
	uint8_t status;

	size_t pmk_len;
	uint8_t pmk[PMK_LEN_MAX];
	bool using_pmksa;

	uint8_t hash[SHA384_MAC_LEN];

	struct wpabuf *beacon_rsne_rsnxe;
	struct wpa_ptk ptk;
	struct crypto_ecdh *ecdh;

	struct wpabuf *comeback;
	uint16_t comeback_after;

#ifdef CONFIG_SAE
	struct sae_data sae;
#endif /* CONFIG_SAE */

#ifdef CONFIG_FILS
	bool fils_eapol;
	bool fils_wd_valid;
	struct pasn_fils fils;
#endif /* CONFIG_FILS */

#ifdef CONFIG_IEEE80211R
	uint8_t pmk_r1[PMK_LEN_MAX];
	size_t pmk_r1_len;
	uint8_t pmk_r1_name[WPA_PMK_NAME_LEN];
#endif /* CONFIG_IEEE80211R */
	/* Note that this pointers to RSN PMKSA cache are actually defined
	 * differently for the PASN initiator (using RSN Supplicant
	 * implementation) and PASN responser (using RSN Authenticator
	 * implementation). Functions cannot be mixed between those cases. */
	struct rsn_pmksa_cache_entry *pmksa_entry;
	struct eapol_sm *eapol;
	int fast_reauth;
#ifdef CONFIG_TESTING_OPTIONS
	int corrupt_mic;
#endif /* CONFIG_TESTING_OPTIONS */
	int network_id;

	uint8_t wrapped_data_format;
	struct wpabuf *secret;

	/* Responder */
	bool noauth; /* Whether PASN without mutual authentication is enabled */
	int disable_pmksa_caching;
	int *pasn_groups;
	struct wpabuf *wrapped_data;
	int use_anti_clogging;
	const uint8_t *rsn_ie;
	size_t rsn_ie_len;

	uint8_t *comeback_key;
	struct os_reltime last_comeback_key_update;
	uint16_t comeback_idx;
	uint16_t *comeback_pending_idx;

	/**
	 * send_mgmt - Function handler to transmit a Management frame
	 * @ctx: Callback context from cb_ctx
	 * @frame_buf : Frame to transmit
	 * @frame_len: Length of frame to transmit
	 * @freq: Frequency in MHz for the channel on which to transmit
	 * @wait_dur: How many milliseconds to wait for a response frame
	 * Returns: 0 on success, -1 on failure
	 */
	int (*send_mgmt)(void *ctx, const uint8_t *data, size_t data_len, int noack,
			 unsigned int freq, unsigned int wait);
	/**
	 * validate_custom_pmkid - Handler to validate vendor specific PMKID
	 * @ctx: Callback context from cb_ctx
	 * @addr : MAC address of the peer
	 * @pmkid: Custom PMKID
	 * Returns: 0 on success (valid PMKID), -1 on failure
	 */
	int (*validate_custom_pmkid)(void *ctx, const uint8_t *addr,
				     const uint8_t *pmkid);
};

/* Initiator */
void wpa_pasn_reset(struct pasn_data *pasn);
int wpas_pasn_start(struct pasn_data *pasn, const uint8_t *own_addr,
		    const uint8_t *peer_addr, const uint8_t *bssid,
		    int akmp, int cipher, uint16_t group,
		    int freq, const uint8_t *beacon_rsne, uint8_t beacon_rsne_len,
		    const uint8_t *beacon_rsnxe, uint8_t beacon_rsnxe_len,
		    const struct wpabuf *comeback);
int wpa_pasn_verify(struct pasn_data *pasn, const uint8_t *own_addr,
		    const uint8_t *peer_addr, const uint8_t *bssid,
		    int akmp, int cipher, uint16_t group,
		    int freq, const uint8_t *beacon_rsne, uint8_t beacon_rsne_len,
		    const uint8_t *beacon_rsnxe, uint8_t beacon_rsnxe_len,
		    const struct wpabuf *comeback);
int wpa_pasn_auth_rx(struct pasn_data *pasn, const uint8_t *data, size_t len,
		     struct wpa_pasn_params_data *pasn_params);
int wpa_pasn_auth_tx_status(struct pasn_data *pasn,
			    const uint8_t *data, size_t data_len, uint8_t acked);

/* Responder */
int handle_auth_pasn_1(struct pasn_data *pasn,
		       const uint8_t *own_addr, const uint8_t *peer_addr,
		       const struct ieee80211_mgmt *mgmt, size_t len);
int handle_auth_pasn_3(struct pasn_data *pasn, const uint8_t *own_addr,
		       const uint8_t *peer_addr,
		       const struct ieee80211_mgmt *mgmt, size_t len);
int handle_auth_pasn_resp(struct pasn_data *pasn, const uint8_t *own_addr,
			  const uint8_t *peer_addr,
			  struct rsn_pmksa_cache_entry *pmksa, uint16_t status);

struct pasn_data * pasn_data_init(void);
void pasn_data_deinit(struct pasn_data *pasn);
void pasn_register_callbacks(struct pasn_data *pasn, void *cb_ctx,
			     int (*send_mgmt)(void *ctx, const uint8_t *data,
					      size_t data_len, int noack,
					      unsigned int freq,
					      unsigned int wait),
			     int (*validate_custom_pmkid)(void *ctx,
							  const uint8_t *addr,
							  const uint8_t *pmkid));
void pasn_enable_kdk_derivation(struct pasn_data *pasn);
void pasn_disable_kdk_derivation(struct pasn_data *pasn);

void pasn_set_akmp(struct pasn_data *pasn, int akmp);
void pasn_set_cipher(struct pasn_data *pasn, int cipher);
void pasn_set_own_addr(struct pasn_data *pasn, const uint8_t *addr);
void pasn_set_peer_addr(struct pasn_data *pasn, const uint8_t *addr);
void pasn_set_bssid(struct pasn_data *pasn, const uint8_t *addr);
void pasn_set_initiator_pmksa(struct pasn_data *pasn,
			      struct rsn_pmksa_cache *pmksa);
void pasn_set_responder_pmksa(struct pasn_data *pasn,
			      struct rsn_pmksa_cache *pmksa);
int pasn_set_pt(struct pasn_data *pasn, struct sae_pt *pt);

/* Responder */
void pasn_set_password(struct pasn_data *pasn, const char *password);
void pasn_set_wpa_key_mgmt(struct pasn_data *pasn, int key_mgmt);
void pasn_set_rsn_pairwise(struct pasn_data *pasn, int rsn_pairwise);
void pasn_set_rsnxe_caps(struct pasn_data *pasn, uint16_t rsnxe_capab);
void pasn_set_rsnxe_ie(struct pasn_data *pasn, const uint8_t *rsnxe_ie);
void pasn_set_custom_pmkid(struct pasn_data *pasn, const uint8_t *pmkid);
int pasn_set_extra_ies(struct pasn_data *pasn, const uint8_t *extra_ies,
		       size_t extra_ies_len);

int pasn_get_akmp(struct pasn_data *pasn);
int pasn_get_cipher(struct pasn_data *pasn);
size_t pasn_get_pmk_len(struct pasn_data *pasn);
uint8_t * pasn_get_pmk(struct pasn_data *pasn);
struct wpa_ptk * pasn_get_ptk(struct pasn_data *pasn);

#ifdef __cplusplus
}
#endif
#endif /* PASN_COMMON_H */
