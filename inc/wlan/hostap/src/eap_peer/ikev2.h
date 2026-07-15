/*
 * IKEv2 responder (RFC 4306) for EAP-IKEV2
 * Copyright (c) 2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_PEER_IKEV2_H
#define INC_WLAN_EAP_PEER_IKEV2_H

#include <types.h>

#include <eap_common/ikev2_common.h>

struct ikev2_proposal_data {
	uint8_t proposal_num;
	int integ;
	int prf;
	int encr;
	int dh;
};


struct ikev2_responder_data {
	enum { SA_INIT, SA_AUTH, CHILD_SA, NOTIFY, IKEV2_DONE, IKEV2_FAILED }
		state;
	uint8_t i_spi[IKEV2_SPI_LEN];
	uint8_t r_spi[IKEV2_SPI_LEN];
	uint8_t i_nonce[IKEV2_NONCE_MAX_LEN];
	size_t i_nonce_len;
	uint8_t r_nonce[IKEV2_NONCE_MAX_LEN];
	size_t r_nonce_len;
	struct wpabuf *i_dh_public;
	struct wpabuf *r_dh_private;
	struct ikev2_proposal_data proposal;
	const struct dh_group *dh;
	struct ikev2_keys keys;
	uint8_t *IDi;
	size_t IDi_len;
	uint8_t IDi_type;
	uint8_t *IDr;
	size_t IDr_len;
	struct wpabuf *r_sign_msg;
	struct wpabuf *i_sign_msg;
	uint8_t *shared_secret;
	size_t shared_secret_len;
	enum { PEER_AUTH_CERT, PEER_AUTH_SECRET } peer_auth;
	uint8_t *key_pad;
	size_t key_pad_len;
	uint16_t error_type;
	enum { LAST_MSG_SA_INIT, LAST_MSG_SA_AUTH } last_msg;
};


void ikev2_responder_deinit(struct ikev2_responder_data *data);
int ikev2_responder_process(struct ikev2_responder_data *data,
			    const struct wpabuf *buf);
struct wpabuf * ikev2_responder_build(struct ikev2_responder_data *data);

#endif /* IKEV2_H */
