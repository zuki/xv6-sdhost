#include <types.h>
#include <string.h>
#include <linux/errno.h>
#include <console.h>
#include <mm.h>

static int sprintint(int64_t x, int base, int sign, int zero, int col, char **p)
{
    static char digit[] = "0123456789abcdef";
    static char buf[64];
    int ret = 0;

    if (sign && x < 0) {
        x = -x;
        *(*p)++ = '-';
    }

    int i = 0;
    uint64_t t = x;
    do {
        buf[i++] = digit[t % base];
    } while (t /= base);
    for (; i < col; i++)
        if (zero == 1)
            buf[i] = '0';
        else if (zero == -1)
            buf[i] = ' ';
        else break;
    ret = i;
    while (i--)
        *(*p)++ = buf[i];
    return ret;
}

static int vsnprintfmt(char *str, size_t size, const char *fmt, va_list ap)
{
    int i, c, j;
    char *s;
    char *p = str;
    int ret = 0;

    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (p - str > size) return -1;
        if (c != '%') {
            *p++ = c;
            ret++;
            continue;
        }

        int n = 0;
        int z = 0;
        if (fmt[i+1] == '0') {
            z = 1;
            i++;
        } else if (fmt[i+1] == '-') {
            z = -1;
            i++;
        }

        for (; fmt[i+1] >= '0' && fmt[i+1] <= '9'; i++) {
            n = n * 10 + (fmt[i+1] - '0') % 10;
        }

        int l = 0;
        for (; fmt[i + 1] == 'l'; i++)
            l++;

        if (!(c = fmt[++i] & 0xff))
            break;

        switch (c) {
        case 'u':
            if (l == 2)
                ret += sprintint(va_arg(ap, int64_t), 10, 0, z, n, &p);
            else
                ret += sprintint(va_arg(ap, uint32_t), 10, 0, z, n, &p);
            break;
        case 'd':
            if (l == 2)
                ret += sprintint(va_arg(ap, int64_t), 10, 1, z, n, &p);
            else
                ret += sprintint(va_arg(ap, int), 10, 1, z, n, &p);
            break;
        case 'x':
            if (l == 2)
                ret += sprintint(va_arg(ap, int64_t), 16, 0, z, n, &p);
            else
                ret += sprintint(va_arg(ap, uint32_t), 16, 0, z, n, &p);
            break;
        case 'p':
            ret += sprintint((uint64_t) va_arg(ap, void *), 16, 0, 0, 16, &p);
            break;
        case 'c':
            *p++ = (va_arg(ap, int));
            ret++;
            break;
        case 's':
            j = 0;
            if ((s = (char*)va_arg(ap, char *)) == 0) {
                s = "(null)";
                j = 6;
            }
            for (; *s; s++) {
                *p++ = *s;
                j++;
                ret++;
            }
            if (n > j) {
                n = n - j;
                while(n--) {
                    *p++ = ' ';
                    ret++;
                }
            }
            break;
        case '%':
            *p++ = '%';
            ret++;
            break;
        default:
            /* Print unknown % sequence to draw attention. */
            *p++ = '%';
            *p++ = c;
            ret += 2;
            break;
        }
    }
    *p = '\0';
    return ret;
}

static int vsprintfmt(char *str, size_t n, const char *fmt, va_list ap)
{
    return vsnprintfmt(str, n, fmt, ap);
}

int snprintf(char *buf, size_t n, const char *fmt, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt);
    rc = vsprintfmt(buf, n, fmt, ap);
    va_end(ap);

    return rc;
}


int sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    int rc;

    va_start(ap, fmt);
    rc = vsprintfmt(buf, 64, fmt, ap);
    va_end(ap);

    return rc;
}

char *strcpy(char *dst, const char *src)
{
    return safestrcpy(dst, src, strlen(src)+1);
}

long strtol(const char *s, char **endptr, int base)
{
    int neg = 0;
    long val = 0;

    // gobble initial whitespace
    while (*s == ' ' || *s == '\t')
        s++;

    // plus/minus sign
    if (*s == '+')
        s++;
    else if (*s == '-')
        s++, neg = 1;

    // hex or octal base prefix
    if ((base == 0 || base == 16) && (s[0] == '0' && s[1] == 'x'))
        s += 2, base = 16;
    else if (base == 0 && s[0] == '0')
        s++, base = 8;
    else if (base == 0)
        base = 10;

    // digits
    while (1) {
        int dig;

        if (*s >= '0' && *s <= '9')
            dig = *s - '0';
        else if (*s >= 'a' && *s <= 'z')
            dig = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'Z')
            dig = *s - 'A' + 10;
        else
            break;
        if (dig >= base)
            break;
        s++, val = (val * base) + dig;
        // we don't properly detect overflow!
    }

    if (endptr)
        *endptr = (char *) s;
    return (neg ? -val : val);
}

long long strtoll(const char *s, char **endptr, int base)
{
    return (long long)strtol(s, endptr, base);
}


unsigned long long strtoull(const char *s, char **endptr, int base)
{
    return (unsigned long long)strtol(s, endptr, base);
}

int atoi(const char *s)
{
    int n = 0, neg = 0;

    while (*s == ' ' || *s == '\t')
        s++;
    switch (*s) {
        case '-':   neg = 1;
        case '+':   s++;
    }

    while (isdigit(*s))
        n = 10 * n - (*s++ - '0');
    return neg ? n : -n;
}

char *strtok_r1(char *s, char delim, char **save_ptr)
{
    char *end;

    if (s == 0)
        s = *save_ptr;

    if (*s == '\0') {
        *save_ptr = s;
        return 0;
    }

    /* Scan leading delimiters.  */
    s += strspn1(s, delim);
    trace("s: 0x%p, s[0]: 0x%02x", s, *s);
    if (*s == '\0') {
        *save_ptr = s;
        return 0;
    }
    /* Find the end of the token.  */
    end = s + strcspn1(s, delim);
    trace("end: 0x%p, end[0]: 0x%02x", end, *end);
    if (*end == '\0')
    {
        *save_ptr = end;
        return s;
    }
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *end = '\0';
    *save_ptr = end + 1;
    trace("*save_ptr: 0x%p, s: %s", *save_ptr, s);
    return s;
}

/* from plan9 : sys/src/ape/lib/bsd/strcasecmp.c */
int strcasecmp(const char *s1, const char *s2)
{
    int c1, c2;

    while (*s1) {
        c1 = *(unsigned char*)s1++;
        c2 = *(unsigned char*)s2++;

        if (c1 == c2)
            continue;

        if (c1 >= 'A' && c1 <= 'Z')
            c1 -= 'A' - 'a';

        if (c2 >= 'A' && c2 <= 'Z')
            c2 -= 'A' - 'a';

        if (c1 != c2)
            return c1 - c2;
    }
    return -*s2;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    int c1, c2;

    while (n > 0 && *s1) {
        c1 = *(unsigned char*)s1++;
        c2 = *(unsigned char*)s2++;

        if (c1 == c2) {
            n--;
            continue;
        }

        if (c1 >= 'A' && c1 <= 'Z')
            c1 -= 'A' - 'a';

        if (c2 >= 'A' && c2 <= 'Z')
            c2 -= 'A' - 'a';

        if (c1 != c2)
            return c1 - c2;
    }
    return n == 0 ? 0 : -*s2;
}

size_t strspn(const char *str, const char *accept)
{
    if (accept[0] == '\0')
        return 0;
    if (accept[1] == '\0') {
        const char *a = str;
        for (; *str == *accept; str++);
        return str - a;
    }

    unsigned char table[256];
    unsigned char *p = memset(table, 0, 256);

    unsigned char *s = (unsigned char *)accept;
    do
        p[*s++] = 1;
    while (*s);

    s = (unsigned char *)str;
    if (!p[s[0]]) return 0;
    if (!p[s[1]]) return 1;
    if (!p[s[2]]) return 2;
    if (!p[s[3]]) return 3;

    s = (unsigned char *)((uintptr_t)s / 4);

    unsigned int c0, c1, c2, c3;
    do {
        s += 4;
        c0 = p[s[0]];
        c1 = p[s[1]];
        c2 = p[s[2]];
        c3 = p[s[3]];
    } while ((c0 & c1 & c2 & c3) != 0);

    size_t count = s - (unsigned char *)str;
    return (c0 & c1) == 0 ? count + c0 : count + c2 + 2;
}

size_t strcspn(const char *str, const char *reject)
{
    if (reject[0] == '\0' || reject[1] == '\0') {
        char *p = strchr(str, reject[0]);
        if (p == NULL)
            p = (char *)((uintptr_t)str + strlen(str));
        return p - str;
    }

    unsigned char table[256];
    unsigned char *p = memset(table, 0, 256);

    unsigned char *s = (unsigned char *)reject;
    unsigned char tmp;
    do
        p[tmp = *s++] = 1;
    while (tmp);

    s = (unsigned char *)str;
    if (p[s[0]]) return 0;
    if (p[s[1]]) return 1;
    if (p[s[2]]) return 2;
    if (p[s[3]]) return 3;

    s = (unsigned char *)((uintptr_t)s / 4);

    unsigned int c0, c1, c2, c3;
    do {
        s += 4;
        c0 = p[s[0]];
        c1 = p[s[1]];
        c2 = p[s[2]];
        c3 = p[s[3]];
    } while ((c0 | c1 | c2 | c3) == 0);

    size_t count = s - (unsigned char *)str;
    return (c0 | c1) != 0 ? count - c0 + 1 : count - c2 + 3;
}

char *strtok_r(char *str, const char *delim, char **save_ptr)
{
    char *end;

    if (str == NULL)
        str = *save_ptr;

    if (*str == '\0') {
        *save_ptr = str;
        return NULL;
    }

    /* Scan leading delimiters.  */
    str += strspn(str, delim);
    trace("s: 0x%p, s[0]: 0x%02x", str, *sstr);
    if (*str == '\0') {
        *save_ptr = str;
        return NULL;
    }
    /* Find the end of the token.  */
    end = str + strcspn(str, delim);
    trace("end: 0x%p, end[0]: 0x%02x", end, *end);
    if (*end == '\0')
    {
        *save_ptr = end;
        return str;
    }
    /* Terminate the token and make *SAVE_PTR point past it.  */
    *end = '\0';
    *save_ptr = end + 1;
    trace("*save_ptr: 0x%p, s: %s", *save_ptr, str);
    return str;
}

// CString::Replaceの挙動を模倣する関数
// 引数:
//   source : 文字列ポインタのポインタ (reallocでアドレスが変わる可能性があるため)
//   find   : 検索する文字列
//   replace: 置換する文字列
// 戻り値: 置換した個数
int str_replace(char **source, const char *find, const char *replace) {
    if (source == NULL || *source == NULL || find == NULL || replace == NULL) {
        return 0;
    }

    size_t find_len = strlen(find);
    if (find_len == 0) {
        return 0; // 検索文字列が空の場合は何もしない
    }
    size_t replace_len = strlen(replace);

    // 1. まず置換が何箇所あるかカウントし、必要なメモリサイズを計算する
    int count = 0;
    const char *tmp = *source;
    while ((tmp = strstr(tmp, find)) != NULL) {
        count++;
        tmp += find_len; // 次の検索へ進む
    }

    // 置換対象が見つからなければ、何もせず0を返す
    if (count == 0) {
        return 0;
    }

    // 2. 新しい文字列の長さを計算してメモリを確保
    size_t source_len = strlen(*source);
    // 新サイズ = 元の長さ + (置換後の長さ - 置換前の長さ) * 置換個数
    size_t new_len = source_len + (replace_len - find_len) * count;

    char *new_str = (char *)kmalloc(new_len + 1);
    if (new_str == NULL) {
        return -1;
    }

    // 3. 文字列の構築（コピーと置換）
    char *dst = new_str;
    const char *src = *source;
    while ((tmp = strstr(src, find)) != NULL) {
        // 検索ヒット位置までの文字列をコピー
        size_t len = tmp - src;
        memcpy(dst, src, len);
        dst += len;

        // 置換文字列をコピー
        memcpy(dst, replace, replace_len);
        dst += replace_len;

        // 次の検索位置へ進む
        src = tmp + find_len;
    }
    // 残りの末尾文字列をコピー
    strncpy(dst, src, strlen(src));

    // 4. 古いメモリを解放し、新しいメモリ空間に差し替える
    kmfree(*source);
    *source = new_str;

    return count; // 置換した個数を返す
}
