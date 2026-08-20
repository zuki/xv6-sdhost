/*
 * WPA Supplicant / Example program entrypoint
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * Modified for Circle by R. Stange <rsta2@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <types.h>
#include <string.h>

#include <utils/common.h>
#include <wpa_supplicant_i.h>
#include <console.h>
#include <net/net.h>
#include <bcm4343.h>

static struct wpa_global *global = NULL;

int wpa_supplicant_is_connected(void)
{
    if (global == NULL || global->ifaces == NULL)
        return 0;

    return global->ifaces->wpa_state == WPA_COMPLETED;
}

int wpa_supplicant_main(const char *confname)
{
    debug("start");
    struct wpa_interface iface;
    int exitcode = 0;
    struct wpa_params params;

    memset(&params, 0, sizeof(params));
    params.wpa_debug_level = MSG_DEBUG;     // これがwpa_supplicantの出力レベル

    global = wpa_supplicant_init(&params);
    if (global == NULL)
        return -1;
    trace("init ok");
    memset(&iface, 0, sizeof(iface));
    iface.driver = "xv6";
    iface.ifname = "wlan0";
    iface.confname = confname;

    if (wpa_supplicant_add_iface(global, &iface, NULL) == NULL)
        exitcode = -1;
    trace("add_iface ok");
    if (exitcode == 0) {
        //bcm4343_dump_status();
        struct net_device *dev = net_device_by_index(NET_INDEX_BCM4343);
        set_ip_config(dev);
        debug("run");
        exitcode = wpa_supplicant_run(global);
    }

    wpa_supplicant_deinit(global);
    global = NULL;

    return exitcode;
}

// in wlan/hostap/wpa_supplicant/wpasupplicant.c
extern void wpasupplicant_init(const char *config_file);

void wpasupplicant_init_thread(void *arg)
{
    wpasupplicant_init("4:/wpa_supplicant.conf");
}
