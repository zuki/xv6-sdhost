#ifndef INC_WLAN_P9UTIL_H
#define INC_WLAN_P9UTIL_H

#include <types.h>
#include <mm.h>
#include <string.h>
#include <console.h>

#define nil                 0

#define USED(var)           ((void) var)

#define nelem(array)        (sizeof (array) / sizeof ((array)[0]))

//#define ROUNDUP(val, num)   (((val)+(num)-1) / (num) * (num))
#define ROUND(val, num)     ROUNDUP(val, num)

//#define MIN(a, b)           ((a) < (b) ? (a) : (b))
//#define MAX(a, b)           ((a) > (b) ? (a) : (b))

#define Mhz                 1000000U
#define sdmalloc            kmalloc
#define sdfree              kmfree

#undef print
#undef sprint
#undef snprint

#define sprint              sprintf
#define snprint             snprintf
#define print               cprintf

char *seprint (char *str, char *end, const char *fmt, ...);

#define readstr             __p9readstr
#define READSTR             1000
long readstr (ulong offset, void *buf, size_t len, const void *p);

#define cistrcmp            strcasecmp
#define cistrncmp           strncasecmp

//#define hexdump             __p9hexdump
//void hexdump (const void *p, size_t len, const char *from);

#endif
