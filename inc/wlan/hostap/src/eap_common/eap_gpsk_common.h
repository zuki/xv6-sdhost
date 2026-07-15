/*
 * EAP server/peer: EAP-GPSK shared routines
 * Copyright (c) 2006-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_GPSK_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_GPSK_COMMON_H

#include <types.h>

#define EAP_GPSK_OPCODE_GPSK_1 1
#define EAP_GPSK_OPCODE_GPSK_2 2
#define EAP_GPSK_OPCODE_GPSK_3 3
#define EAP_GPSK_OPCODE_GPSK_4 4
#define EAP_GPSK_OPCODE_FAIL 5
#define EAP_GPSK_OPCODE_PROTECTED_FAIL 6

/* Failure-Code in GPSK-Fail and GPSK-Protected-Fail */
#define EAP_GPSK_FAIL_PSK_NOT_FOUND 0x00000001
#define EAP_GPSK_FAIL_AUTHENTICATION_FAILURE 0x00000002
#define EAP_GPSK_FAIL_AUTHORIZATION_FAILURE 0x00000003

#define EAP_GPSK_RAND_LEN 32
#define EAP_GPSK_MAX_SK_LEN 32
#define EAP_GPSK_MAX_PK_LEN 32
#define EAP_GPSK_MAX_MIC_LEN 32

#define EAP_GPSK_VENDOR_IETF		0x00000000
#define EAP_GPSK_CIPHER_RESERVED	0x000000
#define EAP_GPSK_CIPHER_AES		0x000001
#define EAP_GPSK_CIPHER_SHA256		0x000002


#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */

struct eap_gpsk_csuite {
	uint8_t vendor[4];
	uint8_t specifier[2];
} STRUCT_PACKED;

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

int eap_gpsk_supported_ciphersuite(int vendor, int specifier);
int eap_gpsk_derive_keys(const uint8_t *psk, size_t psk_len, int vendor,
			 int specifier,
			 const uint8_t *rand_client, const uint8_t *rand_server,
			 const uint8_t *id_client, size_t id_client_len,
			 const uint8_t *id_server, size_t id_server_len,
			 uint8_t *msk, uint8_t *emsk, uint8_t *sk, size_t *sk_len,
			 uint8_t *pk, size_t *pk_len);
int eap_gpsk_derive_session_id(const uint8_t *psk, size_t psk_len, int vendor,
			       int specifier,
			       const uint8_t *rand_peer, const uint8_t *rand_server,
			       const uint8_t *id_peer, size_t id_peer_len,
			       const uint8_t *id_server, size_t id_server_len,
			       uint8_t method_type, uint8_t *sid, size_t *sid_len);
size_t eap_gpsk_mic_len(int vendor, int specifier);
int eap_gpsk_compute_mic(const uint8_t *sk, size_t sk_len, int vendor,
			 int specifier, const uint8_t *data, size_t len, uint8_t *mic);

#endif /* EAP_GPSK_COMMON_H */
