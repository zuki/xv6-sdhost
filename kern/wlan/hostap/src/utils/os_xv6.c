//
// os_circle.cpp
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
#include <utils/os.h>
#include <string.h>
#include <linux/time.h>
#include <clock.h>
#include <random.h>
#include <proc.h>
#include <console.h>

static char vbuf[1024];

int vprintf(const char *fmt, va_list ap)
{
    // char vbuf[1024];
    int rc;
    rc = vsnprintfmt(vbuf, 1024, fmt, ap);
    cprintf(vbuf);
    cprintf1("\n");
    return rc;
}

int printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    int res = vprintf(format, ap);

    va_end(ap);

    return res;
}

// TODO: this is not quick sort
void qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *))
{
    if (nmemb <= 1)
    {
        return;
    }

    assert(base != 0);
    uint8_t *array = (uint8_t *)base;

    assert(size > 0);
    uint8_t tmp[size];

    assert(compare != 0);
    for (unsigned i = 0; i < nmemb - 1; i++)
    {
        for (unsigned j = i + 1; j < nmemb; j++)
        {
#define ELEM(n) &array[(n) * size]
            if ((*compare)(ELEM(i), ELEM(j)) > 0)
            {
                memcpy(tmp, ELEM(i), size);
                memcpy(ELEM(i), ELEM(j), size);
                memcpy(ELEM(j), tmp, size);
            }
        }
    }
}

void abort(void)
{
    panic("aborted");
}

int os_fdatasync(FILE *stream)
{
    return (int)f_sync(stream);
}

void *os_malloc(size_t size)
{
    return kmalloc(size);
}

void *os_realloc(void *ptr, size_t size)
{
    return kmrealloc(ptr, size, strlen((const char *)ptr));
}

void os_free(void *ptr)
{
    return kmfree(ptr);
}

void *os_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

void *os_memmove(void *dest, const void *src, size_t n)
{
    return memmove(dest, src, n);
}

void *os_memset(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

int os_memcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

char *os_strdup(const char *s)
{
    return strdup(s);
}

size_t os_strlen(const char *s)
{
    return strlen(s);
}

int os_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}

int os_strncasecmp(const char *s1, const char *s2, size_t n)
{
    return strncasecmp(s1, s2, n);
}

char *os_strchr(const char *s, int c)
{
    return strchr(s, c);
}

char *os_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

int os_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

int os_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}

char *os_strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

int os_snprintf(char *str, size_t size, const char *fmt, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt);
    rc = vsnprintfmt(str, size, fmt, ap);
    va_end(ap);

    return rc;
}

int os_daemonize(const char *pid_file)
{
    return -1;
}

void os_daemonize_terminate(const char *pid_file)
{
    assert(0);
}

int os_get_random(unsigned char *buf, size_t len)
{
    while (len >= sizeof(uint32_t)) {
        *(uint32_t *)buf = rand();
        buf += sizeof(uint32_t);
        len -= sizeof(uint32_t);
    }

    if (len > 0) {
        uint32_t num = rand();
        switch(len) {
            case 3:
                buf[2] = (num >> 16) & 0xff;
                // fall through
            case 2:
                buf[1] = (num >> 8) & 0xff;
                // fall through
            case 1:
                buf[0] = num & 0xff;
                break;
            default:
                error("bad random");
                assert(0);
                break;
        }
    }

    return 0;
}

unsigned long os_random(void)
{
    return (unsigned long)rand();
}

// TODO: currently expecting absolute path already
char *os_rel2abs_path(const char *rel_path)
{
    if (!rel_path)
    {
        return NULL;
    }

    return strdup(rel_path);
}

int os_get_time(struct os_time *t)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

    t->sec = tp.tv_sec;
    t->usec = tp.tv_nsec * 1000;

    return 0;
}

int os_get_reltime(struct os_reltime *t)
{
    long ticks = get_ticks();           // 1 tick = 10 ms
    t->sec = ticks / HZ;                // HZ = 100
    t->usec = (ticks % HZ) * 10000;

    return 0;
}

void os_sleep(os_time_t sec, os_time_t usec)
{
    if (sec)
    {
        delayus(sec * 1000000);
    }

    if (usec)
    {
        delayus(usec);
    }
}

size_t os_strlcpy(char *dest, const char *src, size_t siz)
{
    safestrcpy(dest, src, siz);
    return strlen(dest);
}

int os_memcmp_const(const void *a, const void *b, size_t len)
{
    const uint8_t *aa = (const uint8_t *)a;
    const uint8_t *bb = (const uint8_t *)b;
    size_t i;
    uint8_t res;

    for (res = 0, i = 0; i < len; i++)
    {
        res |= aa[i] ^ bb[i];
    }

    return res;
}

void *os_memdup(const void *src, size_t len)
{
    void *r = kmalloc(len);

    if (r && src)
    {
        memcpy(r, src, len);
    }

    return r;
}

void *os_zalloc(size_t size)
{
    return kmzalloc(size);
}

void os_hexdump(const char *title, const void *p, size_t len)
{
    unsigned char *src;
    int offset, index;

    src = (unsigned char *)p;

    cprintf("%s: ", title);
    for (offset = 0; offset < (int)len; offset += 16)
    {
        for (index = 0; index < 16; index++)
        {
            if (offset + index < (int)len)
            {
                cprintf("%02x ", 0xff & src[offset + index]);
            }
            else
            {
                cprintf("   ");
            }
        }
#if 0
        cprintf(" = ");
        for (index = 0; index < 16; index++) {
            if (offset + index < (int)len) {
                if (isascii(src[offset + index]) && isprint(src[offset + index])) {
                    cprintf("%c", src[offset + index]);
                } else {
                    cprintf(".");
                }
            } else {
                cprintf(" ");
            }
        }
#endif
        cprintf(" \n");
    }
}
