/*
 * Generic advertisement service (GAS) server
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 * Copyright (c) 2020, The Linux Foundation
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_COMMON_GAS_SERVER_H
#define INC_WLAN_COMMON_GAS_SERVER_H

#ifdef CONFIG_GAS_SERVER

struct gas_server;

struct gas_server * gas_server_init(void *ctx,
				    void (*tx)(void *ctx, int freq,
					       const uint8_t *da,
					       struct wpabuf *buf,
					       unsigned int wait_time));
void gas_server_deinit(struct gas_server *gas);
int gas_server_register(struct gas_server *gas,
			const uint8_t *adv_proto_id, uint8_t adv_proto_id_len,
			struct wpabuf *
			(*req_cb)(void *ctx, void *resp_ctx, const uint8_t *sa,
				  const uint8_t *query, size_t query_len,
				  int *comeback_delay),
			void (*status_cb)(void *ctx, struct wpabuf *resp,
					  int ok),
			void *ctx);
int gas_server_rx(struct gas_server *gas, const uint8_t *da, const uint8_t *sa,
		  const uint8_t *bssid, uint8_t categ, const uint8_t *data, size_t len,
		  int freq);
void gas_server_tx_status(struct gas_server *gas, const uint8_t *dst, const uint8_t *data,
			  size_t data_len, int ack);
int gas_server_set_comeback_delay(struct gas_server *gas, void *resp_ctx,
				  uint16_t comeback_delay);
int gas_server_set_resp(struct gas_server *gas, void *resp_ctx,
			struct wpabuf *resp);
bool gas_server_response_sent(struct gas_server *gas, void *resp_ctx);

#else /* CONFIG_GAS_SERVER */

static inline void gas_server_deinit(struct gas_server *gas)
{
}

#endif /* CONFIG_GAS_SERVER */

#endif /* GAS_SERVER_H */
