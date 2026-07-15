/*
 * Generic advertisement service (GAS) (IEEE 802.11u)
 * Copyright (c) 2009, Atheros Communications
 * Copyright (c) 2011-2012, Qualcomm Atheros
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_COMMON_GAS_H
#define INC_WLAN_COMMON_GAS_H

struct wpabuf * gas_build_initial_req(uint8_t dialog_token, size_t size);
struct wpabuf * gas_build_comeback_req(uint8_t dialog_token);
struct wpabuf * gas_build_initial_resp(uint8_t dialog_token, uint16_t status_code,
				       uint16_t comeback_delay, size_t size);
struct wpabuf *
gas_build_comeback_resp(uint8_t dialog_token, uint16_t status_code, uint8_t frag_id, uint8_t more,
			uint16_t comeback_delay, size_t size);
struct wpabuf * gas_anqp_build_initial_req(uint8_t dialog_token, size_t size);
struct wpabuf * gas_anqp_build_initial_resp(uint8_t dialog_token, uint16_t status_code,
					    uint16_t comeback_delay, size_t size);
struct wpabuf * gas_anqp_build_initial_resp_buf(uint8_t dialog_token,
						uint16_t status_code,
						uint16_t comeback_delay,
						struct wpabuf *payload);
struct wpabuf * gas_anqp_build_comeback_resp(uint8_t dialog_token, uint16_t status_code,
					     uint8_t frag_id, uint8_t more,
					     uint16_t comeback_delay, size_t size);
struct wpabuf * gas_anqp_build_comeback_resp_buf(uint8_t dialog_token,
						 uint16_t status_code,
						 uint8_t frag_id, uint8_t more,
						 uint16_t comeback_delay,
						 struct wpabuf *payload);
void gas_anqp_set_len(struct wpabuf *buf);

uint8_t * gas_anqp_add_element(struct wpabuf *buf, uint16_t info_id);
void gas_anqp_set_element_len(struct wpabuf *buf, uint8_t *len_pos);

#endif /* GAS_H */
