/*
 * wpa_supplicant - WPA definitions
 * Copyright (c) 2003-2015, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_RSN_SUPP_WPA_H
#define INC_WLAN_RSN_SUPP_WPA_H

#include <types.h>
#include <common/defs.h>
#include <common/eapol_common.h>
#include <common/wpa_common.h>
#include <common/ieee802_11_defs.h>

struct wpa_sm;
struct eapol_sm;
struct wpa_config_blob;
struct hostapd_freq_params;
struct wpa_channel_info;
struct rsn_pmksa_cache_entry;
enum frame_encryption;

struct wpa_sm_ctx {
    void *ctx; /* pointer to arbitrary upper level context */
    void *msg_ctx; /* upper level context for wpa_msg() calls */

    void (*set_state)(void *ctx, enum wpa_states state);
    enum wpa_states (*get_state)(void *ctx);
    void (*deauthenticate)(void * ctx, uint16_t reason_code);
    void (*reconnect)(void *ctx);
    int (*set_key)(void *ctx, int link_id, enum wpa_alg alg,
               const uint8_t *addr, int key_idx, int set_tx,
               const uint8_t *seq, size_t seq_len,
               const uint8_t *key, size_t key_len, enum key_flag key_flag);
    void * (*get_network_ctx)(void *ctx);
    int (*get_bssid)(void *ctx, uint8_t *bssid);
    int (*ether_send)(void *ctx, const uint8_t *dest, uint16_t proto, const uint8_t *buf,
              size_t len);
    int (*get_beacon_ie)(void *ctx);
    void (*cancel_auth_timeout)(void *ctx);
    uint8_t * (*alloc_eapol)(void *ctx, uint8_t type, const void *data, uint16_t data_len,
                size_t *msg_len, void **data_pos);
    int (*add_pmkid)(void *ctx, void *network_ctx, const uint8_t *bssid,
             const uint8_t *pmkid, const uint8_t *fils_cache_id,
             const uint8_t *pmk, size_t pmk_len, uint32_t pmk_lifetime,
             uint8_t pmk_reauth_threshold, int akmp);
    int (*remove_pmkid)(void *ctx, void *network_ctx, const uint8_t *bssid,
                const uint8_t *pmkid, const uint8_t *fils_cache_id);
    void (*set_config_blob)(void *ctx, struct wpa_config_blob *blob);
    const struct wpa_config_blob * (*get_config_blob)(void *ctx,
                              const char *name);
    int (*mlme_setprotection)(void *ctx, const uint8_t *addr,
                  int protection_type, int key_type);
    int (*update_ft_ies)(void *ctx, const uint8_t *md, const uint8_t *ies,
                 size_t ies_len);
    int (*send_ft_action)(void *ctx, uint8_t action, const uint8_t *target_ap,
                  const uint8_t *ies, size_t ies_len);
    int (*mark_authenticated)(void *ctx, const uint8_t *target_ap);
#ifdef CONFIG_TDLS
    int (*tdls_get_capa)(void *ctx, int *tdls_supported,
                 int *tdls_ext_setup, int *tdls_chan_switch);
    int (*send_tdls_mgmt)(void *ctx, const uint8_t *dst,
                  uint8_t action_code, uint8_t dialog_token,
                  uint16_t status_code, uint32_t peer_capab,
                  int initiator, const uint8_t *buf, size_t len,
                  int link_id);
    int (*tdls_oper)(void *ctx, int oper, const uint8_t *peer);
    int (*tdls_peer_addset)(void *ctx, const uint8_t *addr, int add, uint16_t aid,
                uint16_t capability, const uint8_t *supp_rates,
                size_t supp_rates_len,
                const struct ieee80211_ht_capabilities *ht_capab,
                const struct ieee80211_vht_capabilities *vht_capab,
                const struct ieee80211_he_capabilities *he_capab,
                size_t he_capab_len,
                const struct ieee80211_he_6ghz_band_cap *he_6ghz_capab,
                uint8_t qosinfo, int wmm, const uint8_t *ext_capab,
                size_t ext_capab_len, const uint8_t *supp_channels,
                size_t supp_channels_len,
                const uint8_t *supp_oper_classes,
                size_t supp_oper_classes_len,
                const struct ieee80211_eht_capabilities *eht_capab,
                size_t eht_capab_len, int mld_link_id);
    int (*tdls_enable_channel_switch)(
        void *ctx, const uint8_t *addr, uint8_t oper_class,
        const struct hostapd_freq_params *params);
    int (*tdls_disable_channel_switch)(void *ctx, const uint8_t *addr);
#endif /* CONFIG_TDLS */
    void (*set_rekey_offload)(void *ctx, const uint8_t *kek, size_t kek_len,
                  const uint8_t *kck, size_t kck_len,
                  const uint8_t *replay_ctr);
    int (*key_mgmt_set_pmk)(void *ctx, const uint8_t *pmk, size_t pmk_len);
    void (*fils_hlp_rx)(void *ctx, const uint8_t *dst, const uint8_t *src,
                const uint8_t *pkt, size_t pkt_len);
    int (*channel_info)(void *ctx, struct wpa_channel_info *ci);
    void (*transition_disable)(void *ctx, uint8_t bitmap);
    void (*store_ptk)(void *ctx, const uint8_t *addr, int cipher,
              uint32_t life_time, const struct wpa_ptk *ptk);
#ifdef CONFIG_PASN
    int (*set_ltf_keyseed)(void *ctx, const uint8_t *own_addr,
                   const uint8_t *peer_addr, size_t ltf_keyseed_len,
                   const uint8_t *ltf_keyseed);
#endif /* CONFIG_PASN */
    void (*notify_pmksa_cache_entry)(void *ctx,
                     struct rsn_pmksa_cache_entry *entry);
    void (*ssid_verified)(void *ctx);
};


enum wpa_sm_conf_params {
    RSNA_PMK_LIFETIME /* dot11RSNAConfigPMKLifetime */,
    RSNA_PMK_REAUTH_THRESHOLD /* dot11RSNAConfigPMKReauthThreshold */,
    RSNA_SA_TIMEOUT /* dot11RSNAConfigSATimeout */,
    WPA_PARAM_PROTO,
    WPA_PARAM_PAIRWISE,
    WPA_PARAM_GROUP,
    WPA_PARAM_KEY_MGMT,
    WPA_PARAM_MGMT_GROUP,
    WPA_PARAM_RSN_ENABLED,
    WPA_PARAM_MFP,
    WPA_PARAM_OCV,
    WPA_PARAM_SAE_PWE,
    WPA_PARAM_SAE_PK,
    WPA_PARAM_DENY_PTK0_REKEY,
    WPA_PARAM_EXT_KEY_ID,
    WPA_PARAM_USE_EXT_KEY_ID,
    WPA_PARAM_FT_RSNXE_USED,
    WPA_PARAM_DPP_PFS,
    WPA_PARAM_WMM_ENABLED,
    WPA_PARAM_OCI_FREQ_EAPOL,
    WPA_PARAM_OCI_FREQ_EAPOL_G2,
    WPA_PARAM_OCI_FREQ_FT_ASSOC,
    WPA_PARAM_OCI_FREQ_FILS_ASSOC,
    WPA_PARAM_DISABLE_EAPOL_G2_TX,
    WPA_PARAM_ENCRYPT_EAPOL_M2,
    WPA_PARAM_ENCRYPT_EAPOL_M4,
    WPA_PARAM_FT_PREPEND_PMKID,
    WPA_PARAM_SSID_PROTECTION,
};

struct rsn_supp_config {
    void *network_ctx;
    int allowed_pairwise_cipher; /* bitfield of WPA_CIPHER_* */
    int proactive_key_caching;
    int eap_workaround;
    void *eap_conf_ctx;
    const uint8_t *ssid;
    size_t ssid_len;
    int wpa_ptk_rekey;
    int wpa_deny_ptk0_rekey;
    int p2p;
    int wpa_rsc_relaxation;
    int owe_ptk_workaround;
    const uint8_t *fils_cache_id;
    int beacon_prot;
    bool force_kdk_derivation;
};

struct wpa_sm_link {
    uint8_t addr[ETH_ALEN];
    uint8_t bssid[ETH_ALEN];
    uint8_t *ap_rsne, *ap_rsnxe;
    size_t ap_rsne_len, ap_rsnxe_len;
    struct wpa_gtk gtk;
    struct wpa_gtk gtk_wnm_sleep;
    struct wpa_igtk igtk;
    struct wpa_igtk igtk_wnm_sleep;
    struct wpa_bigtk bigtk;
    struct wpa_bigtk bigtk_wnm_sleep;
};

struct wpa_sm_mlo {
    uint8_t ap_mld_addr[ETH_ALEN];
    uint8_t assoc_link_id;
    uint16_t valid_links; /* bitmap of accepted links */
    uint16_t req_links; /* bitmap of requested links */
    struct wpa_sm_link links[MAX_NUM_MLD_LINKS];
};

#ifndef CONFIG_NO_WPA

struct wpa_sm * wpa_sm_init(struct wpa_sm_ctx *ctx);
void wpa_sm_deinit(struct wpa_sm *sm);
void wpa_sm_notify_assoc(struct wpa_sm *sm, const uint8_t *bssid);
void wpa_sm_notify_disassoc(struct wpa_sm *sm);
void wpa_sm_set_pmk(struct wpa_sm *sm, const uint8_t *pmk, size_t pmk_len,
            const uint8_t *pmkid, const uint8_t *bssid);
void wpa_sm_set_pmk_from_pmksa(struct wpa_sm *sm);
void wpa_sm_set_fast_reauth(struct wpa_sm *sm, int fast_reauth);
void wpa_sm_set_scard_ctx(struct wpa_sm *sm, void *scard_ctx);
void wpa_sm_set_config(struct wpa_sm *sm, struct rsn_supp_config *config);
void wpa_sm_set_ssid(struct wpa_sm *sm, const uint8_t *ssid, size_t ssid_len);
void wpa_sm_set_own_addr(struct wpa_sm *sm, const uint8_t *addr);
void wpa_sm_set_ifname(struct wpa_sm *sm, const char *ifname,
               const char *bridge_ifname);
void wpa_sm_set_eapol(struct wpa_sm *sm, struct eapol_sm *eapol);
int wpa_sm_set_assoc_wpa_ie(struct wpa_sm *sm, const uint8_t *ie, size_t len);
int wpa_sm_set_assoc_wpa_ie_default(struct wpa_sm *sm, uint8_t *wpa_ie,
                    size_t *wpa_ie_len);
int wpa_sm_set_assoc_rsnxe_default(struct wpa_sm *sm, uint8_t *rsnxe,
                   size_t *rsnxe_len);
int wpa_sm_set_assoc_rsnxe(struct wpa_sm *sm, const uint8_t *ie, size_t len);
int wpa_sm_set_ap_wpa_ie(struct wpa_sm *sm, const uint8_t *ie, size_t len);
int wpa_sm_set_ap_rsn_ie(struct wpa_sm *sm, const uint8_t *ie, size_t len);
int wpa_sm_set_ap_rsnxe(struct wpa_sm *sm, const uint8_t *ie, size_t len);
int wpa_sm_get_mib(struct wpa_sm *sm, char *buf, size_t buflen);

int wpa_sm_set_param(struct wpa_sm *sm, enum wpa_sm_conf_params param,
             unsigned int value);

int wpa_sm_get_status(struct wpa_sm *sm, char *buf, size_t buflen,
              int verbose);
int wpa_sm_pmf_enabled(struct wpa_sm *sm);
int wpa_sm_ext_key_id(struct wpa_sm *sm);
int wpa_sm_ext_key_id_active(struct wpa_sm *sm);
int wpa_sm_ocv_enabled(struct wpa_sm *sm);

void wpa_sm_key_request(struct wpa_sm *sm, int error, int pairwise);

int wpa_parse_wpa_ie(const uint8_t *wpa_ie, size_t wpa_ie_len,
             struct wpa_ie_data *data);

void wpa_sm_aborted_cached(struct wpa_sm *sm);
void wpa_sm_aborted_external_cached(struct wpa_sm *sm);
int wpa_sm_rx_eapol(struct wpa_sm *sm, const uint8_t *src_addr,
            const uint8_t *buf, size_t len, enum frame_encryption encrypted);
int wpa_sm_parse_own_wpa_ie(struct wpa_sm *sm, struct wpa_ie_data *data);
int wpa_sm_pmksa_cache_list(struct wpa_sm *sm, char *buf, size_t len);
struct rsn_pmksa_cache_entry * wpa_sm_pmksa_cache_head(struct wpa_sm *sm);
struct rsn_pmksa_cache_entry *
wpa_sm_pmksa_cache_add_entry(struct wpa_sm *sm,
                 struct rsn_pmksa_cache_entry * entry);
void wpa_sm_pmksa_cache_add(struct wpa_sm *sm, const uint8_t *pmk, size_t pmk_len,
                const uint8_t *pmkid, const uint8_t *bssid,
                const uint8_t *fils_cache_id);
int wpa_sm_pmksa_exists(struct wpa_sm *sm, const uint8_t *bssid, const uint8_t *own_addr,
            const void *network_ctx);
void wpa_sm_drop_sa(struct wpa_sm *sm);
struct rsn_pmksa_cache_entry * wpa_sm_pmksa_cache_get(struct wpa_sm *sm,
                              const uint8_t *aa,
                              const uint8_t *pmkid,
                              const void *network_ctx,
                              int akmp);
void wpa_sm_pmksa_cache_remove(struct wpa_sm *sm,
                   struct rsn_pmksa_cache_entry *entry);
bool wpa_sm_has_ft_keys(struct wpa_sm *sm, const uint8_t *md);
int wpa_sm_has_ptk_installed(struct wpa_sm *sm);

void wpa_sm_update_replay_ctr(struct wpa_sm *sm, const uint8_t *replay_ctr);

void wpa_sm_pmksa_cache_flush(struct wpa_sm *sm, void *network_ctx);
void wpa_sm_external_pmksa_cache_flush(struct wpa_sm *sm, void *network_ctx);

int wpa_sm_get_p2p_ip_addr(struct wpa_sm *sm, uint8_t *buf);

void wpa_sm_set_rx_replay_ctr(struct wpa_sm *sm, const uint8_t *rx_replay_counter);
void wpa_sm_set_ptk_kck_kek(struct wpa_sm *sm,
                const uint8_t *ptk_kck, size_t ptk_kck_len,
                const uint8_t *ptk_kek, size_t ptk_kek_len);
int wpa_fils_is_completed(struct wpa_sm *sm);
void wpa_sm_pmksa_cache_reconfig(struct wpa_sm *sm);
int wpa_sm_set_mlo_params(struct wpa_sm *sm, const struct wpa_sm_mlo *mlo);

#else /* CONFIG_NO_WPA */

static inline struct wpa_sm * wpa_sm_init(struct wpa_sm_ctx *ctx)
{
    return (struct wpa_sm *) 1;
}

static inline void wpa_sm_deinit(struct wpa_sm *sm)
{
}

static inline void wpa_sm_notify_assoc(struct wpa_sm *sm, const uint8_t *bssid)
{
}

static inline void wpa_sm_notify_disassoc(struct wpa_sm *sm)
{
}

static inline void wpa_sm_set_pmk(struct wpa_sm *sm, const uint8_t *pmk,
                  size_t pmk_len, const uint8_t *pmkid,
                  const uint8_t *bssid)
{
}

static inline void wpa_sm_set_pmk_from_pmksa(struct wpa_sm *sm)
{
}

static inline void wpa_sm_set_fast_reauth(struct wpa_sm *sm, int fast_reauth)
{
}

static inline void wpa_sm_set_scard_ctx(struct wpa_sm *sm, void *scard_ctx)
{
}

static inline void wpa_sm_set_config(struct wpa_sm *sm,
                     struct rsn_supp_config *config)
{
}

static inline void wpa_sm_set_own_addr(struct wpa_sm *sm, const uint8_t *addr)
{
}

static inline void wpa_sm_set_ifname(struct wpa_sm *sm, const char *ifname,
                     const char *bridge_ifname)
{
}

static inline void wpa_sm_set_eapol(struct wpa_sm *sm, struct eapol_sm *eapol)
{
}

static inline int wpa_sm_set_assoc_wpa_ie(struct wpa_sm *sm, const uint8_t *ie,
                      size_t len)
{
    return -1;
}

static inline int wpa_sm_set_assoc_wpa_ie_default(struct wpa_sm *sm,
                          uint8_t *wpa_ie,
                          size_t *wpa_ie_len)
{
    return -1;
}

static inline int wpa_sm_set_ap_wpa_ie(struct wpa_sm *sm, const uint8_t *ie,
                       size_t len)
{
    return -1;
}

static inline int wpa_sm_set_ap_rsn_ie(struct wpa_sm *sm, const uint8_t *ie,
                       size_t len)
{
    return -1;
}

static inline int wpa_sm_set_ap_rsnxe(struct wpa_sm *sm, const uint8_t *ie,
                      size_t len)
{
    return -1;
}

static inline int wpa_sm_get_mib(struct wpa_sm *sm, char *buf, size_t buflen)
{
    return 0;
}

static inline int wpa_sm_set_param(struct wpa_sm *sm,
                   enum wpa_sm_conf_params param,
                   unsigned int value)
{
    return -1;
}

static inline int wpa_sm_get_status(struct wpa_sm *sm, char *buf,
                    size_t buflen, int verbose)
{
    return 0;
}

static inline int wpa_sm_pmf_enabled(struct wpa_sm *sm)
{
    return 0;
}

static inline int wpa_sm_ext_key_id(struct wpa_sm *sm)
{
    return 0;
}

static inline int wpa_sm_ext_key_id_active(struct wpa_sm *sm)
{
    return 0;
}

static inline int wpa_sm_ocv_enabled(struct wpa_sm *sm)
{
    return 0;
}

static inline void wpa_sm_key_request(struct wpa_sm *sm, int error,
                      int pairwise)
{
}

static inline int wpa_parse_wpa_ie(const uint8_t *wpa_ie, size_t wpa_ie_len,
                   struct wpa_ie_data *data)
{
    return -1;
}

static inline void wpa_sm_aborted_cached(struct wpa_sm *sm)
{
}

static inline void wpa_sm_aborted_external_cached(struct wpa_sm *sm)
{
}

static inline int wpa_sm_rx_eapol(struct wpa_sm *sm, const uint8_t *src_addr,
                  const uint8_t *buf, size_t len,
                  enum frame_encryption encrypted)
{
    return -1;
}

static inline int wpa_sm_parse_own_wpa_ie(struct wpa_sm *sm,
                      struct wpa_ie_data *data)
{
    return -1;
}

static inline int wpa_sm_pmksa_cache_list(struct wpa_sm *sm, char *buf,
                      size_t len)
{
    return -1;
}

static inline void wpa_sm_drop_sa(struct wpa_sm *sm)
{
}

static inline struct rsn_pmksa_cache_entry *
wpa_sm_pmksa_cache_get(struct wpa_sm *sm, const uint8_t *aa, const uint8_t *pmkid,
               const void *network_ctx, int akmp)
{
    return NULL;
}

static inline int wpa_sm_has_ptk(struct wpa_sm *sm)
{
    return 0;
}

static inline void wpa_sm_update_replay_ctr(struct wpa_sm *sm,
                        const uint8_t *replay_ctr)
{
}

static inline void wpa_sm_external_pmksa_cache_flush(struct wpa_sm *sm,
                             void *network_ctx)
{
}

static inline void wpa_sm_pmksa_cache_flush(struct wpa_sm *sm,
                        void *network_ctx)
{
}

static inline void wpa_sm_set_rx_replay_ctr(struct wpa_sm *sm,
                        const uint8_t *rx_replay_counter)
{
}

static inline void wpa_sm_set_ptk_kck_kek(struct wpa_sm *sm, const uint8_t *ptk_kck,
                      size_t ptk_kck_len,
                      const uint8_t *ptk_kek, size_t ptk_kek_len)
{
}

static inline int wpa_fils_is_completed(struct wpa_sm *sm)
{
    return 0;
}

static inline void wpa_sm_pmksa_cache_reconfig(struct wpa_sm *sm)
{
}

static inline int wpa_sm_set_mlo_params(struct wpa_sm *sm,
                    const struct wpa_sm_mlo *mlo)
{
    return 0;
}

#endif /* CONFIG_NO_WPA */

#ifdef CONFIG_IEEE80211R

int wpa_sm_set_ft_params(struct wpa_sm *sm, const uint8_t *ies, size_t ies_len);
int wpa_ft_prepare_auth_request(struct wpa_sm *sm, const uint8_t *mdie);
int wpa_ft_add_mdie(struct wpa_sm *sm, uint8_t *ies, size_t ies_len,
            const uint8_t *mdie);
const uint8_t * wpa_sm_get_ft_md(struct wpa_sm *sm);
int wpa_ft_process_response(struct wpa_sm *sm, const uint8_t *ies, size_t ies_len,
                int ft_action, const uint8_t *target_ap,
                const uint8_t *ric_ies, size_t ric_ies_len);
int wpa_ft_is_completed(struct wpa_sm *sm);
void wpa_reset_ft_completed(struct wpa_sm *sm);
int wpa_ft_validate_reassoc_resp(struct wpa_sm *sm, const uint8_t *ies,
                 size_t ies_len, const uint8_t *src_addr);
int wpa_ft_start_over_ds(struct wpa_sm *sm, const uint8_t *target_ap,
             const uint8_t *mdie, bool force);

#ifdef CONFIG_PASN

int wpa_pasn_ft_derive_pmk_r1(struct wpa_sm *sm, int akmp, const uint8_t *r1kh_id,
                  uint8_t *pmk_r1, size_t *pmk_r1_len, uint8_t *pmk_r1_name);

#endif /* CONFIG_PASN */

#else /* CONFIG_IEEE80211R */

static inline int
wpa_sm_set_ft_params(struct wpa_sm *sm, const uint8_t *ies, size_t ies_len)
{
    return 0;
}

static inline int wpa_ft_prepare_auth_request(struct wpa_sm *sm,
                          const uint8_t *mdie)
{
    return 0;
}

static inline int wpa_ft_add_mdie(struct wpa_sm *sm, uint8_t *ies, size_t ies_len,
                  const uint8_t *mdie)
{
    return 0;
}

static inline int
wpa_ft_process_response(struct wpa_sm *sm, const uint8_t *ies, size_t ies_len,
            int ft_action, const uint8_t *target_ap)
{
    return 0;
}

static inline int wpa_ft_is_completed(struct wpa_sm *sm)
{
    return 0;
}

static inline void wpa_reset_ft_completed(struct wpa_sm *sm)
{
}

static inline int
wpa_ft_validate_reassoc_resp(struct wpa_sm *sm, const uint8_t *ies, size_t ies_len,
                 const uint8_t *src_addr)
{
    return -1;
}

#ifdef CONFIG_PASN

static inline int
wpa_pasn_ft_derive_pmk_r1(struct wpa_sm *sm, int akmp, const uint8_t *r1kh_id,
              uint8_t *pmk_r1, size_t *pmk_r1_len, uint8_t *pmk_r1_name)
{
    return -1;
}

#endif /* CONFIG_PASN */

#endif /* CONFIG_IEEE80211R */


/* tdls.c */
void wpa_tdls_ap_ies(struct wpa_sm *sm, const uint8_t *ies, size_t len);
void wpa_tdls_assoc_resp_ies(struct wpa_sm *sm, const uint8_t *ies, size_t len);
int wpa_tdls_start(struct wpa_sm *sm, const uint8_t *addr);
void wpa_tdls_remove(struct wpa_sm *sm, const uint8_t *addr);
int wpa_tdls_teardown_link(struct wpa_sm *sm, const uint8_t *addr, uint16_t reason_code);
int wpa_tdls_send_discovery_request(struct wpa_sm *sm, const uint8_t *addr);
int wpa_tdls_init(struct wpa_sm *sm);
void wpa_tdls_teardown_peers(struct wpa_sm *sm);
void wpa_tdls_deinit(struct wpa_sm *sm);
void wpa_tdls_enable(struct wpa_sm *sm, int enabled);
void wpa_tdls_disable_unreachable_link(struct wpa_sm *sm, const uint8_t *addr);
const char * wpa_tdls_get_link_status(struct wpa_sm *sm, const uint8_t *addr);
int wpa_tdls_is_external_setup(struct wpa_sm *sm);
int wpa_tdls_enable_chan_switch(struct wpa_sm *sm, const uint8_t *addr,
                uint8_t oper_class,
                struct hostapd_freq_params *freq_params);
int wpa_tdls_disable_chan_switch(struct wpa_sm *sm, const uint8_t *addr);
int wpa_tdls_process_discovery_response(struct wpa_sm *sm, const uint8_t *addr,
                    const uint8_t *buf, size_t len);
#ifdef CONFIG_TDLS_TESTING
extern unsigned int tdls_testing;
#endif /* CONFIG_TDLS_TESTING */


int wpa_wnmsleep_install_key(struct wpa_sm *sm, uint8_t subelem_id, uint8_t *buf);
void wpa_sm_set_test_assoc_ie(struct wpa_sm *sm, struct wpabuf *buf);
void wpa_sm_set_test_eapol_m2_elems(struct wpa_sm *sm, struct wpabuf *buf);
void wpa_sm_set_test_eapol_m4_elems(struct wpa_sm *sm, struct wpabuf *buf);
const uint8_t * wpa_sm_get_anonce(struct wpa_sm *sm);
unsigned int wpa_sm_get_key_mgmt(struct wpa_sm *sm);

struct wpabuf * fils_build_auth(struct wpa_sm *sm, int dh_group, const uint8_t *md);
int fils_process_auth(struct wpa_sm *sm, const uint8_t *bssid, const uint8_t *data,
              size_t len);
struct wpabuf * fils_build_assoc_req(struct wpa_sm *sm, const uint8_t **kek,
                     size_t *kek_len, const uint8_t **snonce,
                     const uint8_t **anonce,
                     const struct wpabuf **hlp,
                     unsigned int num_hlp);
int fils_process_assoc_resp(struct wpa_sm *sm, const uint8_t *resp, size_t len);

struct wpabuf * owe_build_assoc_req(struct wpa_sm *sm, uint16_t group);
int owe_process_assoc_resp(struct wpa_sm *sm, const uint8_t *bssid,
               const uint8_t *resp_ies, size_t resp_ies_len);

void wpa_sm_set_reset_fils_completed(struct wpa_sm *sm, int set);
void wpa_sm_set_fils_cache_id(struct wpa_sm *sm, const uint8_t *fils_cache_id);
void wpa_sm_set_dpp_z(struct wpa_sm *sm, const struct wpabuf *z);
void wpa_pasn_sm_set_caps(struct wpa_sm *sm, unsigned int flags2);
struct rsn_pmksa_cache * wpa_sm_get_pmksa_cache(struct wpa_sm *sm);

void wpa_sm_set_cur_pmksa(struct wpa_sm *sm,
              struct rsn_pmksa_cache_entry *entry);
const uint8_t * wpa_sm_get_auth_addr(struct wpa_sm *sm);
void wpa_sm_set_driver_bss_selection(struct wpa_sm *sm,
                     bool driver_bss_selection);

#endif /* WPA_H */
