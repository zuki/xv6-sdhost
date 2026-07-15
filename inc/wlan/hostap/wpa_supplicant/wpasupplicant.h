//
// wpasupplicant.h
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
#ifndef INC_WPA_SUPPLICANT_WPASUPPLICANT_H
#define INC_WPA_SUPPLICANT_WPASUPPLICANT_H

#include <types.h>
#include <net/platform.h>

struct wpasupplicant {
    char *config_file;
    struct sched_ctx ctx;
};

void wpasupplicant_init(const char *config_file);
void wpasupplicant_deinit(struct wpasupplicant *self);
static void proc_entry(void *param);

#endif
