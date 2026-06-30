#ifndef INC_STRING_H
#define INC_STRING_H

#include <types.h>
#include <arm.h>

static inline void *
memset(void *str, int c, size_t n)
{
    char *l = (char *)str, *r = l + n;
    for (; l != r; l ++)
        *l = (char)(c & 0xff);
    return str;
}

static inline void *
memmove(void *dst, const void *src, size_t n)
{
    const char *s = (const char *)src;
    char *d = (char *)dst;
    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (n-- > 0) *--d = *--s;
    } else {
        while (n-- > 0) *d++ = *s++;
    }
    return dst;
}

static inline void *
memcpy(void *dst, const void *src, ssize_t n)
{
    return memmove(dst, src, n);
}

static inline int
memcmp(const void *v1, const void *v2, size_t n)
{
    for (const uint8_t *s1 = (const uint8_t*)v1, *s2 = (const uint8_t*)v2;
         n-- > 0; s1++, s2++)
        if  (*s1 != *s2)
            return *s1 - *s2;
    return 0;
}

static inline int
strncmp(const char *p, const char *q, size_t n)
{
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (uint8_t)*p - (uint8_t)*q;
}

static inline char *
strncpy(char *dst, const char *src, size_t n)
{
    if (n != 0) {
        char *d = dst;
        const char *s = src;
        do {
            if ((*d++ = *s++) == 0) {
                while (--n != 0)
                    *d++ = 0;
                break;
            }
        } while (--n != 0);
    }
    return (dst);
}

// Like strncpy but guaranteed to NUL-terminate.
static inline char *
safestrcpy(char *s, const char *t, size_t n)
{
    char *os = s;
    if (n <= 0)
        return os;
    while (--n > 0 && (*s++ = *t++) != 0)
        ;
    *s = 0;
    return os;
}

static inline size_t
strlen(const char *s)
{
    size_t n;
    for (n = 0; s[n]; n++)
        ;
    return n;
}

static inline char *
strchr(const char *s, char c)
{
    if (s == 0)
        return 0;

    char *p = (char *)s;
    while (*p != '\0' && *p != c)
        p++;

    return *p == c ? p : 0;
}

static inline char *
strrchr(const char *s, char c)
{
    char *p = (char *)((uint64_t)s + strlen(s) - 1);
    for (; *p && p >= s; p--) {
        if (*p == c)
            return (char *)p;
    }
    return 0;
}

static inline int
strspn1(const char *s, char c)
{
    if (s == 0)
        return 0;

    char *p = (char *)s;
    while(*p == c)
        p++;

    return (int)(p - s);
}

static inline int
strcspn1(const char *s, char c)
{
    if (s == 0)
        return 0;

    char *p = (char *)s;
    while(*p != c && *p != '\0')
        p++;

    return (int)(p - s);
}

static inline int
strcmp(const char *p, const char *q)
{
    while (*p && *p == *q)
        p++, q++;
    return (int) ((uint8_t)*p - (uint8_t)*q);
}

static inline void *
memscan(void *addr, int c, size_t size)
{
    unsigned char *a = addr;

    while (size) {
        if (*a == c)
            return (void *)a;
        a++;
        size--;
    }
    return (void *)a;
}

/* from plan9 : sys/src/libc/port/strstr.c */
static inline char *strstr(const char *s1, const char *s2)
{
    char *p, *pa, *pb;
    int c0, c;

    c0 = *s2;
    if (c0 == 0)
        return s1;
    s2++;
    for (p = strchr(s1, c0); p; p = strchr(p+1, c0)) {
        pa = p;
        for (pb = s2; ; pb++) {
            c = *pb;
            if (c == 0)
                return p;
            if (c != *++pa)
                break;
        }
    }
    return 0;
}

static inline void format(uint8_t c, char *sc) {
    const char digit[] = "0123456789abcdef";
    sc[0] = digit[c >> 4];
    sc[1] = digit[c & 0xf];
}

int sprintf(char *str, const char *fmt, ...);

static __inline int __isspace(int _c)
{
    return _c == ' ' || (unsigned)_c-'\t' < 5;
}

#define isascii(a) ((a >= 0x00) && (a <= 0x7f))
#define isprint(a) ((a >= 0x20) && (a <= 0x7e))

#define isalpha(a) ((((unsigned)(a)|32)-'a') < 26)
#define isdigit(a) (((unsigned)(a)-'0') < 10)
#define islower(a) (((unsigned)(a)-'a') < 26)
#define isupper(a) (((unsigned)(a)-'A') < 26)
#define isgraph(a) (((unsigned)(a)-0x21) < 0x5e)
#define issapce(a) __isspace(a)

char *strcpy(char *dst, const char *src);
int atoi(const char *s);
long strtol(const char *s, char **endptr, int base);
long long strtoll(const char *s, char **endptr, int base);
unsigned long long strtoull(const char *s, char **endptr, int base);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t n, const char *fmt, ...);
char *strtok_r1(char *s, char delim, char **save_ptr);
size_t strspn(const char *str, const char *accept);
size_t strcspn(const char *str, const char *reject);
char *strtok_r(char *str, const char *delim, char **save_ptr);
int str_replace(char **source, const char *find, const char *replace);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif
