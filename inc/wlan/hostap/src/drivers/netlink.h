/*
 * Netlink helper functions for driver wrappers
 * Copyright (c) 2002-2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_DRIVERS_NETLINK_H
#define INC_WLAN_DRIVERS_NETLINK_H

#include <types.h>

struct netlink_data;
struct ifinfomsg;

struct netlink_config {
	void *ctx;
	void (*newlink_cb)(void *ctx, struct ifinfomsg *ifi, uint8_t *buf,
			   size_t len);
	void (*dellink_cb)(void *ctx, struct ifinfomsg *ifi, uint8_t *buf,
			   size_t len);
};

struct netlink_data * netlink_init(struct netlink_config *cfg);
void netlink_deinit(struct netlink_data *netlink);
int netlink_send_oper_ifla(struct netlink_data *netlink, int ifindex,
			   int linkmode, int operstate);

#endif /* NETLINK_H */
