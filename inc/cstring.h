#ifndef INC_CSTRING_H
#define INC_CSTRING_H

#include <types.h>

// 可変長文字列を管理する構造体
typedef struct cstring {
    char    *data;       // 文字列データ
    size_t  length;    // 現在の文字列長
    size_t  capacity;  // 確保しているメモリのサイズ
} cstring_t;

int cstring_init(cstring_t *str, const char *value);
int cstring_append(cstring_t *str, const char *value);
int cstring_replace(cstring_t *str, const char *find, const char *replace);
int cstring_truncate(cstring_t *str);
void cstring_free(cstring_t *str);

#endif
