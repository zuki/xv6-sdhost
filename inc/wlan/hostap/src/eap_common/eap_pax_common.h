/*
 * EAP server/peer: EAP-PAX shared routines
 * Copyright (c) 2005-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_PAX_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_PAX_COMMON_H

#include <types.h>

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

struct eap_pax_hdr {
	uint8_t op_code;
	uint8_t flags;
	uint8_t mac_id;
	uint8_t dh_group_id;
	uint8_t public_key_id;
	/* Followed by variable length payload and ICV */
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */


/* op_code: */
enum {
	EAP_PAX_OP_STD_1 = 0x01,
	EAP_PAX_OP_STD_2 = 0x02,
	EAP_PAX_OP_STD_3 = 0x03,
	EAP_PAX_OP_SEC_1 = 0x11,
	EAP_PAX_OP_SEC_2 = 0x12,
	EAP_PAX_OP_SEC_3 = 0x13,
	EAP_PAX_OP_SEC_4 = 0x14,
	EAP_PAX_OP_SEC_5 = 0x15,
	EAP_PAX_OP_ACK = 0x21
};

/* flags: */
#define EAP_PAX_FLAGS_MF			0x01
#define EAP_PAX_FLAGS_CE			0x02
#define EAP_PAX_FLAGS_AI			0x04

/* mac_id: */
#define EAP_PAX_MAC_HMAC_SHA1_128		0x01
#define EAP_PAX_HMAC_SHA256_128			0x02

/* dh_group_id: */
#define EAP_PAX_DH_GROUP_NONE			0x00
#define EAP_PAX_DH_GROUP_2048_MODP		0x01
#define EAP_PAX_DH_GROUP_3072_MODP		0x02
#define EAP_PAX_DH_GROUP_NIST_ECC_P_256		0x03

/* public_key_id: */
#define EAP_PAX_PUBLIC_KEY_NONE			0x00
#define EAP_PAX_PUBLIC_KEY_RSAES_OAEP		0x01
#define EAP_PAX_PUBLIC_KEY_RSA_PKCS1_V1_5	0x02
#define EAP_PAX_PUBLIC_KEY_EL_GAMAL_NIST_ECC	0x03

/* ADE type: */
#define EAP_PAX_ADE_VENDOR_SPECIFIC		0x01
#define EAP_PAX_ADE_CLIENT_CHANNEL_BINDING	0x02
#define EAP_PAX_ADE_SERVER_CHANNEL_BINDING	0x03


#define EAP_PAX_RAND_LEN 32
#define EAP_PAX_MAC_LEN 16
#define EAP_PAX_ICV_LEN 16
#define EAP_PAX_AK_LEN 16
#define EAP_PAX_MK_LEN 16
#define EAP_PAX_CK_LEN 16
#define EAP_PAX_ICK_LEN 16
#define EAP_PAX_MID_LEN 16


int eap_pax_kdf(uint8_t mac_id, const uint8_t *key, size_t key_len,
		const char *identifier,
		const uint8_t *entropy, size_t entropy_len,
		size_t output_len, uint8_t *output);
int eap_pax_mac(uint8_t mac_id, const uint8_t *key, size_t key_len,
		const uint8_t *data1, size_t data1_len,
		const uint8_t *data2, size_t data2_len,
		const uint8_t *data3, size_t data3_len,
		uint8_t *mac);
int eap_pax_initial_key_derivation(uint8_t mac_id, const uint8_t *ak, const uint8_t *e,
				   uint8_t *mk, uint8_t *ck, uint8_t *ick, uint8_t *mid);

#endif /* EAP_PAX_COMMON_H */
