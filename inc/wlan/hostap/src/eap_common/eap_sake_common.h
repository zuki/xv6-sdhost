/*
 * EAP server/peer: EAP-SAKE shared routines
 * Copyright (c) 2006-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_SAKE_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_SAKE_COMMON_H

#include <types.h>

#define EAP_SAKE_VERSION 2

#define EAP_SAKE_SUBTYPE_CHALLENGE 1
#define EAP_SAKE_SUBTYPE_CONFIRM 2
#define EAP_SAKE_SUBTYPE_AUTH_REJECT 3
#define EAP_SAKE_SUBTYPE_IDENTITY 4

#define EAP_SAKE_AT_RAND_S 1
#define EAP_SAKE_AT_RAND_P 2
#define EAP_SAKE_AT_MIC_S 3
#define EAP_SAKE_AT_MIC_P 4
#define EAP_SAKE_AT_SERVERID 5
#define EAP_SAKE_AT_PEERID 6
#define EAP_SAKE_AT_SPI_S 7
#define EAP_SAKE_AT_SPI_P 8
#define EAP_SAKE_AT_ANY_ID_REQ 9
#define EAP_SAKE_AT_PERM_ID_REQ 10
#define EAP_SAKE_AT_ENCR_DATA 128
#define EAP_SAKE_AT_IV 129
#define EAP_SAKE_AT_PADDING 130
#define EAP_SAKE_AT_NEXT_TMPID 131
#define EAP_SAKE_AT_MSK_LIFE 132

#define EAP_SAKE_RAND_LEN 16
#define EAP_SAKE_MIC_LEN 16
#define EAP_SAKE_ROOT_SECRET_LEN 16
#define EAP_SAKE_SMS_LEN 16
#define EAP_SAKE_TEK_AUTH_LEN 16
#define EAP_SAKE_TEK_CIPHER_LEN 16
#define EAP_SAKE_TEK_LEN (EAP_SAKE_TEK_AUTH_LEN + EAP_SAKE_TEK_CIPHER_LEN)

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

struct eap_sake_hdr {
	uint8_t version; /* EAP_SAKE_VERSION */
	uint8_t session_id;
	uint8_t subtype;
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */


struct eap_sake_parse_attr {
	const uint8_t *rand_s;
	const uint8_t *rand_p;
	const uint8_t *mic_s;
	const uint8_t *mic_p;
	const uint8_t *serverid;
	size_t serverid_len;
	const uint8_t *peerid;
	size_t peerid_len;
	const uint8_t *spi_s;
	size_t spi_s_len;
	const uint8_t *spi_p;
	size_t spi_p_len;
	const uint8_t *any_id_req;
	const uint8_t *perm_id_req;
	const uint8_t *encr_data;
	size_t encr_data_len;
	const uint8_t *iv;
	size_t iv_len;
	const uint8_t *next_tmpid;
	size_t next_tmpid_len;
	const uint8_t *msk_life;
};

int eap_sake_parse_attributes(const uint8_t *buf, size_t len,
			      struct eap_sake_parse_attr *attr);
int eap_sake_derive_keys(const uint8_t *root_secret_a, const uint8_t *root_secret_b,
			 const uint8_t *rand_s, const uint8_t *rand_p,
			 uint8_t *tek, uint8_t *msk, uint8_t *emsk);
int eap_sake_compute_mic(const uint8_t *tek_auth,
			 const uint8_t *rand_s, const uint8_t *rand_p,
			 const uint8_t *serverid, size_t serverid_len,
			 const uint8_t *peerid, size_t peerid_len,
			 int peer, const uint8_t *eap, size_t eap_len,
			 const uint8_t *mic_pos, uint8_t *mic);
void eap_sake_add_attr(struct wpabuf *buf, uint8_t type, const uint8_t *data,
		       size_t len);

#endif /* EAP_SAKE_COMMON_H */
