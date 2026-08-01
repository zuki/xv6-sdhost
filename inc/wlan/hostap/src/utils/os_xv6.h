//
// os_circle.h
//
// OS specific functions for Circle
// by R. Stange <rsta2@gmx.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//
#ifndef INC_WLAN_UTILS_OS_XV6_H
#define INC_WLAN_UTILS_OS_XV6_H

#include <types.h>

#define __LITTLE_ENDIAN     1234
#define __BYTE_ORDER        __LITTLE_ENDIAN
#define WPA_TYPES_DEFINED

//#define NULL        0

#define __force

#undef  ENOMEM
#undef  EINVAL
#undef  EBUSY
#undef  EAGAIN
#undef  EIO
#undef  EOPNOTSUPP
#undef  ENOTCONN
#undef  ECANCELED
#define ENOMEM          1
#define EINVAL          2
#define EBUSY           3
#define EAGAIN          4
#define EIO             5
#define EOPNOTSUPP      6
#define ENOTCONN        7
#define ECANCELED       8

#define bswap16     __builtin_bswap16
#define bswap32     __builtin_bswap32

#define bswap_16    bswap16
#define bswap_32    bswap32

struct in_addr
{
    uint32_t    s_addr;
};

struct ip6_addr
{
    union {
            uint8_t     __s6_addr[16];
            uint16_t    __s6_addr16[8];
            uint32_t    __s6_addr32[4];
    } __in6_union;
};

#define qsort       __wpa_qsort
#define abort       __wpa_abort
#define printf      __wpa_printf
#define vprintf     __wpa_vprintf

int printf(const char *format, ...);
int vprintf(const char *format, va_list ap, boolean nl);

void qsort (void *base, size_t nmemb, size_t size,
        int (*compare) (const void *, const void *));

void abort(void);

void os_hexdump(const char *title, const void *p, size_t len);

#endif
