//
// wpasupplicant.cpp
//
// WPA Supplicant wrapper class for Circle
// by R. Stange <rsta2@gmx.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//
#include <types.h>
#include <wpasupplicant.h>
#include <net/net.h>
#include <proc.h>
#include <wlan/bcm4343.h>
#include <utils/eloop.h>
#include <console.h>

extern int wpa_supplicant_main(const char *confname);
extern int wpa_supplicant_is_connected(void);

static struct wpasupplicant wapsupplicant;

static void proc_entry(void *param)
{
    struct wpasupplicant *self = (struct wpasupplicant *)param;
    assert(self != 0);

    wpa_supplicant_main(self->config_file);

    sched_interrupt(&self->ctx);
}

static boolean is_connected(void)
{
    return !!wpa_supplicant_is_connected();
}

static boolean initialize(struct wpasupplicant *self)
{
    struct net_device *dev = net_device_by_index(NET_INDEX_BCM4343);
    assert(dev != 0);
    struct bcm4343 *bcm4343_dev = (struct bcm4343 *)dev;
    bcm4343_dev->is_connected = is_connected;
    kthread_create("wpa_supplicant", proc_entry, self, 32);

    return true;
}

void wpasupplicant_init(const char *config_file)
{
    wapsupplicant.config_file = config_file;
    sched_ctx_init(&wapsupplicant.ctx);
    initialize(&wapsupplicant);
}

void wpasupplicant_deinit(struct wpasupplicant *self)
{
    eloop_terminate();
    if (sched_ctx_destroy(&self->ctx) == -1) {
        sched_wakeup(&self->ctx);
        return;
    }
}
