/*
 * Wi-Fi Protected Setup - attribute parsing
 * Copyright (c) 2008-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_WPS_WPS_ATTR_PARSE_H
#define INC_WLAN_WPS_WPS_ATTR_PARSE_H

#include <types.h>
#include <wps/wps.h>

struct wps_parse_attr {
	/* fixed length fields */
	const uint8_t *version; /* 1 octet */
	const uint8_t *version2; /* 1 octet */
	const uint8_t *msg_type; /* 1 octet */
	const uint8_t *enrollee_nonce; /* WPS_NONCE_LEN (16) octets */
	const uint8_t *registrar_nonce; /* WPS_NONCE_LEN (16) octets */
	const uint8_t *uuid_r; /* WPS_UUID_LEN (16) octets */
	const uint8_t *uuid_e; /* WPS_UUID_LEN (16) octets */
	const uint8_t *auth_type_flags; /* 2 octets */
	const uint8_t *encr_type_flags; /* 2 octets */
	const uint8_t *conn_type_flags; /* 1 octet */
	const uint8_t *config_methods; /* 2 octets */
	const uint8_t *sel_reg_config_methods; /* 2 octets */
	const uint8_t *primary_dev_type; /* 8 octets */
	const uint8_t *rf_bands; /* 1 octet */
	const uint8_t *assoc_state; /* 2 octets */
	const uint8_t *config_error; /* 2 octets */
	const uint8_t *dev_password_id; /* 2 octets */
	const uint8_t *os_version; /* 4 octets */
	const uint8_t *wps_state; /* 1 octet */
	const uint8_t *authenticator; /* WPS_AUTHENTICATOR_LEN (8) octets */
	const uint8_t *r_hash1; /* WPS_HASH_LEN (32) octets */
	const uint8_t *r_hash2; /* WPS_HASH_LEN (32) octets */
	const uint8_t *e_hash1; /* WPS_HASH_LEN (32) octets */
	const uint8_t *e_hash2; /* WPS_HASH_LEN (32) octets */
	const uint8_t *r_snonce1; /* WPS_SECRET_NONCE_LEN (16) octets */
	const uint8_t *r_snonce2; /* WPS_SECRET_NONCE_LEN (16) octets */
	const uint8_t *e_snonce1; /* WPS_SECRET_NONCE_LEN (16) octets */
	const uint8_t *e_snonce2; /* WPS_SECRET_NONCE_LEN (16) octets */
	const uint8_t *key_wrap_auth; /* WPS_KWA_LEN (8) octets */
	const uint8_t *auth_type; /* 2 octets */
	const uint8_t *encr_type; /* 2 octets */
	const uint8_t *network_idx; /* 1 octet */
	const uint8_t *network_key_idx; /* 1 octet */
	const uint8_t *mac_addr; /* ETH_ALEN (6) octets */
	const uint8_t *selected_registrar; /* 1 octet (Bool) */
	const uint8_t *request_type; /* 1 octet */
	const uint8_t *response_type; /* 1 octet */
	const uint8_t *ap_setup_locked; /* 1 octet */
	const uint8_t *settings_delay_time; /* 1 octet */
	const uint8_t *network_key_shareable; /* 1 octet (Bool) */
	const uint8_t *request_to_enroll; /* 1 octet (Bool) */
	const uint8_t *ap_channel; /* 2 octets */
	const uint8_t *registrar_configuration_methods; /* 2 octets */

	/* variable length fields */
	const uint8_t *manufacturer;
	const uint8_t *model_name;
	const uint8_t *model_number;
	const uint8_t *serial_number;
	const uint8_t *dev_name;
	const uint8_t *public_key;
	const uint8_t *encr_settings;
	const uint8_t *ssid; /* <= 32 octets */
	const uint8_t *network_key; /* <= 64 octets */
	const uint8_t *authorized_macs; /* <= 30 octets */
	const uint8_t *sec_dev_type_list; /* <= 128 octets */
	const uint8_t *oob_dev_password; /* 38..54 octets */
	uint16_t manufacturer_len;
	uint16_t model_name_len;
	uint16_t model_number_len;
	uint16_t serial_number_len;
	uint16_t dev_name_len;
	uint16_t public_key_len;
	uint16_t encr_settings_len;
	uint16_t ssid_len;
	uint16_t network_key_len;
	uint16_t authorized_macs_len;
	uint16_t sec_dev_type_list_len;
	uint16_t oob_dev_password_len;

	/* attributes that can occur multiple times */
#define MAX_CRED_COUNT 10
#define MAX_REQ_DEV_TYPE_COUNT 10

	unsigned int num_cred;
	unsigned int num_req_dev_type;
	unsigned int num_vendor_ext;

	uint16_t cred_len[MAX_CRED_COUNT];
	uint16_t vendor_ext_len[MAX_WPS_PARSE_VENDOR_EXT];

	const uint8_t *cred[MAX_CRED_COUNT];
	const uint8_t *req_dev_type[MAX_REQ_DEV_TYPE_COUNT];
	const uint8_t *vendor_ext[MAX_WPS_PARSE_VENDOR_EXT];
	uint8_t multi_ap_ext;
};

int wps_parse_msg(const struct wpabuf *msg, struct wps_parse_attr *attr);

#endif /* WPS_ATTR_PARSE_H */
