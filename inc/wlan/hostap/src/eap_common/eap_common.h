/*
 * EAP common peer/server definitions
 * Copyright (c) 2004-2014, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_COMMON_H

#include <types.h>
#include <utils/wpabuf.h>

struct erp_tlvs {
	const uint8_t *keyname;
	const uint8_t *domain;

	uint8_t keyname_len;
	uint8_t domain_len;
};

int eap_hdr_len_valid(const struct wpabuf *msg, size_t min_payload);
const uint8_t * eap_hdr_validate(int vendor, enum eap_type eap_type,
			    const struct wpabuf *msg, size_t *plen);
struct wpabuf * eap_msg_alloc(int vendor, enum eap_type type,
			      size_t payload_len, uint8_t code, uint8_t identifier);
void eap_update_len(struct wpabuf *msg);
uint8_t eap_get_id(const struct wpabuf *msg);
enum eap_type eap_get_type(const struct wpabuf *msg);
int erp_parse_tlvs(const uint8_t *pos, const uint8_t *end, struct erp_tlvs *tlvs,
		   int stop_at_keyname);

#endif /* EAP_COMMON_H */
