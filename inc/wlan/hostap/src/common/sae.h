/*
 * Simultaneous authentication of equals
 * Copyright (c) 2012-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_COMMON_SAE_H
#define INC_WLAN_COMMON_SAE_H

#define SAE_KCK_LEN 32
#define SAE_PMK_LEN 32
#define SAE_PMK_LEN_MAX 64
#define SAE_PMKID_LEN 16
#define SAE_MAX_PRIME_LEN 512
#define SAE_MAX_ECC_PRIME_LEN 66
#define SAE_MAX_HASH_LEN 64
#define SAE_COMMIT_MAX_LEN (2 + 3 * SAE_MAX_PRIME_LEN + 255)
#ifdef CONFIG_SAE_PK
#define SAE_CONFIRM_MAX_LEN ((2 + SAE_MAX_HASH_LEN) + 1500)
#else /* CONFIG_SAE_PK */
#define SAE_CONFIRM_MAX_LEN (2 + SAE_MAX_HASH_LEN)
#endif /* CONFIG_SAE_PK */
#define SAE_PK_M_LEN 16

/* Special value returned by sae_parse_commit() */
#define SAE_SILENTLY_DISCARD 65535

struct sae_pk {
	struct wpabuf *m;
	struct crypto_ec_key *key;
	int group;
	struct wpabuf *pubkey; /* DER encoded subjectPublicKey */
#ifdef CONFIG_TESTING_OPTIONS
	struct crypto_ec_key *sign_key_override;
#endif /* CONFIG_TESTING_OPTIONS */
};


struct sae_temporary_data {
	uint8_t kck[SAE_MAX_HASH_LEN];
	size_t kck_len;
	struct crypto_bignum *own_commit_scalar;
	struct crypto_bignum *own_commit_element_ffc;
	struct crypto_ec_point *own_commit_element_ecc;
	struct crypto_bignum *peer_commit_element_ffc;
	struct crypto_ec_point *peer_commit_element_ecc;
	struct crypto_ec_point *pwe_ecc;
	struct crypto_bignum *pwe_ffc;
	struct crypto_bignum *sae_rand;
	struct crypto_ec *ec;
	int prime_len;
	int order_len;
	const struct dh_group *dh;
	const struct crypto_bignum *prime;
	const struct crypto_bignum *order;
	struct crypto_bignum *prime_buf;
	struct crypto_bignum *order_buf;
	struct wpabuf *anti_clogging_token;
	char *pw_id;
	int vlan_id;
	uint8_t bssid[ETH_ALEN];
	struct wpabuf *own_rejected_groups;
	struct wpabuf *peer_rejected_groups;
	unsigned int own_addr_higher:1;

#ifdef CONFIG_SAE_PK
	uint8_t kek[SAE_MAX_HASH_LEN];
	size_t kek_len;
	const struct sae_pk *ap_pk;
	uint8_t own_addr[ETH_ALEN];
	uint8_t peer_addr[ETH_ALEN];
	uint8_t fingerprint[SAE_MAX_HASH_LEN];
	size_t fingerprint_bytes;
	size_t fingerprint_bits;
	size_t lambda;
	unsigned int sec;
	uint8_t ssid[32];
	size_t ssid_len;
#ifdef CONFIG_TESTING_OPTIONS
	bool omit_pk_elem;
#endif /* CONFIG_TESTING_OPTIONS */
#endif /* CONFIG_SAE_PK */

	struct os_reltime disabled_until;
};

struct sae_pt {
	struct sae_pt *next;
	int group;
	struct crypto_ec *ec;
	struct crypto_ec_point *ecc_pt;

	const struct dh_group *dh;
	struct crypto_bignum *ffc_pt;
#ifdef CONFIG_SAE_PK
	uint8_t ssid[32];
	size_t ssid_len;
#endif /* CONFIG_SAE_PK */
};

enum sae_state {
	SAE_NOTHING, SAE_COMMITTED, SAE_CONFIRMED, SAE_ACCEPTED
};

struct sae_data {
	enum sae_state state;
	uint16_t send_confirm;
	uint8_t pmk[SAE_PMK_LEN_MAX];
	size_t pmk_len;
	int akmp; /* WPA_KEY_MGMT_* used in key derivation */
	uint32_t own_akm_suite_selector;
	uint32_t peer_akm_suite_selector;
	uint8_t pmkid[SAE_PMKID_LEN];
	struct crypto_bignum *peer_commit_scalar;
	struct crypto_bignum *peer_commit_scalar_accepted;
	int group;
	unsigned int sync; /* protocol instance variable: Sync */
	uint16_t rc; /* protocol instance variable: Rc (received send-confirm) */
	unsigned int h2e:1;
	unsigned int pk:1;
	struct sae_temporary_data *tmp;
};

int sae_set_group(struct sae_data *sae, int group);
void sae_clear_temp_data(struct sae_data *sae);
void sae_clear_data(struct sae_data *sae);

int sae_prepare_commit(const uint8_t *addr1, const uint8_t *addr2,
		       const uint8_t *password, size_t password_len,
		       struct sae_data *sae);
int sae_prepare_commit_pt(struct sae_data *sae, const struct sae_pt *pt,
			  const uint8_t *addr1, const uint8_t *addr2,
			  int *rejected_groups, const struct sae_pk *pk);
int sae_process_commit(struct sae_data *sae);
int sae_write_commit(struct sae_data *sae, struct wpabuf *buf,
		     const struct wpabuf *token, const char *identifier);
uint16_t sae_parse_commit(struct sae_data *sae, const uint8_t *data, size_t len,
		     const uint8_t **token, size_t *token_len, int *allowed_groups,
		     int h2e, int *ie_offset);
int sae_write_confirm(struct sae_data *sae, struct wpabuf *buf);
int sae_check_confirm(struct sae_data *sae, const uint8_t *data, size_t len,
		      int *ie_offset);
uint16_t sae_group_allowed(struct sae_data *sae, int *allowed_groups, uint16_t group);
const char * sae_state_txt(enum sae_state state);
size_t sae_ecc_prime_len_2_hash_len(size_t prime_len);
size_t sae_ffc_prime_len_2_hash_len(size_t prime_len);
struct sae_pt * sae_derive_pt(int *groups, const uint8_t *ssid, size_t ssid_len,
			      const uint8_t *password, size_t password_len,
			      const char *identifier);
struct crypto_ec_point *
sae_derive_pwe_from_pt_ecc(const struct sae_pt *pt,
			   const uint8_t *addr1, const uint8_t *addr2);
struct crypto_bignum *
sae_derive_pwe_from_pt_ffc(const struct sae_pt *pt,
			   const uint8_t *addr1, const uint8_t *addr2);
void sae_deinit_pt(struct sae_pt *pt);

/* sae_pk.c */
#ifdef CONFIG_SAE_PK
bool sae_pk_valid_password(const char *pw);
#else /* CONFIG_SAE_PK */
static inline bool sae_pk_valid_password(const char *pw)
{
	return false;
}
#endif /* CONFIG_SAE_PK */
char * sae_pk_base32_encode(const uint8_t *src, size_t len_bits);
uint8_t * sae_pk_base32_decode(const char *src, size_t len, size_t *out_len);
int sae_pk_set_password(struct sae_data *sae, const char *password);
void sae_deinit_pk(struct sae_pk *pk);
struct sae_pk * sae_parse_pk(const char *val);
int sae_write_confirm_pk(struct sae_data *sae, struct wpabuf *buf);
int sae_check_confirm_pk(struct sae_data *sae, const uint8_t *ies, size_t ies_len);
int sae_hash(size_t hash_len, const uint8_t *data, size_t len, uint8_t *hash);
uint32_t sae_pk_get_be19(const uint8_t *buf);
void sae_pk_buf_shift_left_19(uint8_t *buf, size_t len);

#endif /* SAE_H */
