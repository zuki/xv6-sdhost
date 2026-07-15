/*
 * wpa_supplicant/hostapd - Default include files
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This header file is included into all C files so that commonly used header
 * files can be selected with OS specific ifdef blocks in one place instead of
 * having to have OS/C library specific selection in many files.
 */

#ifndef INC_WLAN_UTILS_INCLUDES_H
#define INC_WLAN_UTILS_INCLUDES_H

/* Include possible build time configuration before including anything else */
#include <utils/build_config.h>

#include <utils/os_xv6.h>

#endif /* INCLUDES_H */
