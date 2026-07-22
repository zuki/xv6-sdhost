#include <types.h>
#include <wlan/p9util.h>
#include <string.h>

char *seprint(char *str, char *end, const char *fmt, ...)
{
    va_list ap;
    int rc;

    assert (str < end);
    size_t len = end - str - 1;

    va_start (ap, fmt);
    rc = vsnprintfmt(str, len, fmt, ap);
    va_end(ap);

    return str + (strlen(str) < len ? strlen(str) : len);
}

long readstr(ulong offset, void *buf, size_t len, const void *p)
{
    const char *p1 = (const char *) p;

    size_t plen = strlen(p1);
    if (offset >= plen) {
        return 0;
    }

    p1 += offset;

    char *p2 = (char *) buf;
    long result = 0;

    while (*p1 != '\0' && len > 0) {
        *p2++ = *p1++;

        result++;
        len--;
    }

    return result;
}


