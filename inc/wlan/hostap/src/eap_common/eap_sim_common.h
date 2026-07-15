/*
 * EAP peer/server: EAP-SIM/AKA/AKA' shared routines
 * Copyright (c) 2004-2008, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_SIM_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_SIM_COMMON_H

#include <types.h>

#define EAP_SIM_NONCE_S_LEN 16
#define EAP_SIM_NONCE_MT_LEN 16
#define EAP_SIM_MAC_LEN 16
#define EAP_SIM_MK_LEN 20
#define EAP_SIM_K_AUT_LEN 16
#define EAP_SIM_K_ENCR_LEN 16
#define EAP_SIM_KEYING_DATA_LEN 64
#define EAP_SIM_IV_LEN 16
#define EAP_SIM_KC_LEN 8
#define EAP_SIM_SRES_LEN 4

#define GSM_RAND_LEN 16

#define EAP_SIM_VERSION 1

/* EAP-SIM Subtypes */
#define EAP_SIM_SUBTYPE_START 10
#define EAP_SIM_SUBTYPE_CHALLENGE 11
#define EAP_SIM_SUBTYPE_NOTIFICATION 12
#define EAP_SIM_SUBTYPE_REAUTHENTICATION 13
#define EAP_SIM_SUBTYPE_CLIENT_ERROR 14

/* AT_CLIENT_ERROR_CODE error codes */
#define EAP_SIM_UNABLE_TO_PROCESS_PACKET 0
#define EAP_SIM_UNSUPPORTED_VERSION 1
#define EAP_SIM_INSUFFICIENT_NUM_OF_CHAL 2
#define EAP_SIM_RAND_NOT_FRESH 3

#define EAP_SIM_MAX_FAST_REAUTHS 1000

#define EAP_SIM_MAX_CHAL 3


/* EAP-AKA Subtypes */
#define EAP_AKA_SUBTYPE_CHALLENGE 1
#define EAP_AKA_SUBTYPE_AUTHENTICATION_REJECT 2
#define EAP_AKA_SUBTYPE_SYNCHRONIZATION_FAILURE 4
#define EAP_AKA_SUBTYPE_IDENTITY 5
#define EAP_AKA_SUBTYPE_NOTIFICATION 12
#define EAP_AKA_SUBTYPE_REAUTHENTICATION 13
#define EAP_AKA_SUBTYPE_CLIENT_ERROR 14

/* AT_CLIENT_ERROR_CODE error codes */
#define EAP_AKA_UNABLE_TO_PROCESS_PACKET 0

#define EAP_AKA_RAND_LEN 16
#define EAP_AKA_AUTN_LEN 16
#define EAP_AKA_AUTS_LEN 14
#define EAP_AKA_RES_MAX_LEN 16
#define EAP_AKA_IK_LEN 16
#define EAP_AKA_CK_LEN 16
#define EAP_AKA_MAX_FAST_REAUTHS 1000
#define EAP_AKA_MIN_RES_LEN 4
#define EAP_AKA_MAX_RES_LEN 16
#define EAP_AKA_CHECKCODE_LEN 20

#define EAP_AKA_PRIME_K_AUT_LEN 32
#define EAP_AKA_PRIME_CHECKCODE_LEN 32
#define EAP_AKA_PRIME_K_RE_LEN 32

struct wpabuf;

void eap_sim_derive_mk(const uint8_t *identity, size_t identity_len,
		       const uint8_t *nonce_mt, uint16_t selected_version,
		       const uint8_t *ver_list, size_t ver_list_len,
		       int num_chal, const uint8_t *kc, uint8_t *mk);
void eap_aka_derive_mk(const uint8_t *identity, size_t identity_len,
		       const uint8_t *ik, const uint8_t *ck, uint8_t *mk);
int eap_sim_derive_keys(const uint8_t *mk, uint8_t *k_encr, uint8_t *k_aut, uint8_t *msk,
			uint8_t *emsk);
int eap_sim_derive_keys_reauth(uint16_t _counter,
			       const uint8_t *identity, size_t identity_len,
			       const uint8_t *nonce_s, const uint8_t *mk, uint8_t *msk,
			       uint8_t *emsk);
int eap_sim_verify_mac(const uint8_t *k_aut, const struct wpabuf *req,
		       const uint8_t *mac, const uint8_t *extra, size_t extra_len);
void eap_sim_add_mac(const uint8_t *k_aut, const uint8_t *msg, size_t msg_len, uint8_t *mac,
		     const uint8_t *extra, size_t extra_len);

#if defined(EAP_AKA_PRIME) || defined(EAP_SERVER_AKA_PRIME)
void eap_aka_prime_derive_keys(const uint8_t *identity, size_t identity_len,
			       const uint8_t *ik, const uint8_t *ck, uint8_t *k_encr,
			       uint8_t *k_aut, uint8_t *k_re, uint8_t *msk, uint8_t *emsk);
int eap_aka_prime_derive_keys_reauth(const uint8_t *k_re, uint16_t counter,
				     const uint8_t *identity, size_t identity_len,
				     const uint8_t *nonce_s, uint8_t *msk, uint8_t *emsk);
int eap_sim_verify_mac_sha256(const uint8_t *k_aut, const struct wpabuf *req,
			      const uint8_t *mac, const uint8_t *extra,
			      size_t extra_len);
void eap_sim_add_mac_sha256(const uint8_t *k_aut, const uint8_t *msg, size_t msg_len,
			    uint8_t *mac, const uint8_t *extra, size_t extra_len);

void eap_aka_prime_derive_ck_ik_prime(uint8_t *ck, uint8_t *ik, const uint8_t *sqn_ak,
				      const uint8_t *network_name,
				      size_t network_name_len);
#else /* EAP_AKA_PRIME || EAP_SERVER_AKA_PRIME */
static inline void eap_aka_prime_derive_keys(const uint8_t *identity,
					     size_t identity_len,
					     const uint8_t *ik, const uint8_t *ck,
					     uint8_t *k_encr, uint8_t *k_aut, uint8_t *k_re,
					     uint8_t *msk, uint8_t *emsk)
{
}

static inline int eap_aka_prime_derive_keys_reauth(const uint8_t *k_re, uint16_t counter,
						   const uint8_t *identity,
						   size_t identity_len,
						   const uint8_t *nonce_s, uint8_t *msk,
						   uint8_t *emsk)
{
	return -1;
}

static inline int eap_sim_verify_mac_sha256(const uint8_t *k_aut,
					    const struct wpabuf *req,
					    const uint8_t *mac, const uint8_t *extra,
					    size_t extra_len)
{
	return -1;
}
#endif /* EAP_AKA_PRIME || EAP_SERVER_AKA_PRIME */


/* EAP-SIM/AKA Attributes (0..127 non-skippable) */
#define EAP_SIM_AT_RAND 1
#define EAP_SIM_AT_AUTN 2 /* only AKA */
#define EAP_SIM_AT_RES 3 /* only AKA, only peer->server */
#define EAP_SIM_AT_AUTS 4 /* only AKA, only peer->server */
#define EAP_SIM_AT_PADDING 6 /* only encrypted */
#define EAP_SIM_AT_NONCE_MT 7 /* only SIM, only send */
#define EAP_SIM_AT_PERMANENT_ID_REQ 10
#define EAP_SIM_AT_MAC 11
#define EAP_SIM_AT_NOTIFICATION 12
#define EAP_SIM_AT_ANY_ID_REQ 13
#define EAP_SIM_AT_IDENTITY 14 /* only send */
#define EAP_SIM_AT_VERSION_LIST 15 /* only SIM */
#define EAP_SIM_AT_SELECTED_VERSION 16 /* only SIM */
#define EAP_SIM_AT_FULLAUTH_ID_REQ 17
#define EAP_SIM_AT_COUNTER 19 /* only encrypted */
#define EAP_SIM_AT_COUNTER_TOO_SMALL 20 /* only encrypted */
#define EAP_SIM_AT_NONCE_S 21 /* only encrypted */
#define EAP_SIM_AT_CLIENT_ERROR_CODE 22 /* only send */
#define EAP_SIM_AT_KDF_INPUT 23 /* only AKA' */
#define EAP_SIM_AT_KDF 24 /* only AKA' */
#define EAP_SIM_AT_IV 129
#define EAP_SIM_AT_ENCR_DATA 130
#define EAP_SIM_AT_NEXT_PSEUDONYM 132 /* only encrypted */
#define EAP_SIM_AT_NEXT_REAUTH_ID 133 /* only encrypted */
#define EAP_SIM_AT_CHECKCODE 134 /* only AKA */
#define EAP_SIM_AT_RESULT_IND 135
#define EAP_SIM_AT_BIDDING 136

/* AT_NOTIFICATION notification code values */
#define EAP_SIM_GENERAL_FAILURE_AFTER_AUTH 0
#define EAP_SIM_TEMPORARILY_DENIED 1026
#define EAP_SIM_NOT_SUBSCRIBED 1031
#define EAP_SIM_GENERAL_FAILURE_BEFORE_AUTH 16384
#define EAP_SIM_SUCCESS 32768

/* EAP-AKA' AT_KDF Key Derivation Function values */
#define EAP_AKA_PRIME_KDF 1

/* AT_BIDDING flags */
#define EAP_AKA_BIDDING_FLAG_D 0x8000


enum eap_sim_id_req {
	NO_ID_REQ, ANY_ID, FULLAUTH_ID, PERMANENT_ID
};


struct eap_sim_attrs {
	const uint8_t *rand, *autn, *mac, *iv, *encr_data, *version_list, *nonce_s;
	const uint8_t *next_pseudonym, *next_reauth_id;
	const uint8_t *nonce_mt, *identity, *res, *auts;
	const uint8_t *checkcode;
	const uint8_t *kdf_input;
	const uint8_t *bidding;
	size_t num_chal, version_list_len, encr_data_len;
	size_t next_pseudonym_len, next_reauth_id_len, identity_len, res_len;
	size_t res_len_bits;
	size_t checkcode_len;
	size_t kdf_input_len;
	enum eap_sim_id_req id_req;
	int notification, counter, selected_version, client_error_code;
	int counter_too_small;
	int result_ind;
#define EAP_AKA_PRIME_KDF_MAX 10
	uint16_t kdf[EAP_AKA_PRIME_KDF_MAX];
	size_t kdf_count;
};

int eap_sim_parse_attr(const uint8_t *start, const uint8_t *end,
		       struct eap_sim_attrs *attr, int aka, int encr);
uint8_t * eap_sim_parse_encr(const uint8_t *k_encr, const uint8_t *encr_data,
			size_t encr_data_len, const uint8_t *iv,
			struct eap_sim_attrs *attr, int aka);


struct eap_sim_msg;

struct eap_sim_msg * eap_sim_msg_init(int code, int id, int type, int subtype);
struct wpabuf * eap_sim_msg_finish(struct eap_sim_msg *msg, int type,
				   const uint8_t *k_aut,
				   const uint8_t *extra, size_t extra_len);
void eap_sim_msg_free(struct eap_sim_msg *msg);
uint8_t * eap_sim_msg_add_full(struct eap_sim_msg *msg, uint8_t attr,
			  const uint8_t *data, size_t len);
uint8_t * eap_sim_msg_add(struct eap_sim_msg *msg, uint8_t attr,
		     uint16_t value, const uint8_t *data, size_t len);
uint8_t * eap_sim_msg_add_mac(struct eap_sim_msg *msg, uint8_t attr);
int eap_sim_msg_add_encr_start(struct eap_sim_msg *msg, uint8_t attr_iv,
			       uint8_t attr_encr);
int eap_sim_msg_add_encr_end(struct eap_sim_msg *msg, uint8_t *k_encr,
			     int attr_pad);

void eap_sim_report_notification(void *msg_ctx, int notification, int aka);
int eap_sim_anonymous_username(const uint8_t *id, size_t id_len);

#endif /* EAP_SIM_COMMON_H */
