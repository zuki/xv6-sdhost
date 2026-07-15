#include <types.h>
#include <cstring.h>
#include <string.h>
#include <mm.h>
#include <console.h>

// 初期化関数
int cstring_init(cstring_t *str, const char *value) {
    size_t len = value ? strlen(value) : 0;
    str->capacity = len + 1; // ヌル文字分を含む
    str->data = (char *)kmalloc(str->capacity);
    if (str->data) {
        safestrcpy(str->data, value, len);
        str->length = len;
    } else {
        return -1;
    }
    return 0;
}

// 文字列の末尾追加関数 (CString::Append 相当)
int cstring_append(cstring_t *str, const char *value) {
    if (!value || value[0] == '\0') return 0;

    size_t append_len = strlen(value);
    size_t new_len = str->length + append_len;

    // バッファが不足している場合、メモリを拡張する
    if (new_len >= str->capacity) {
        // 容量を倍々に増やしていく（一般的な最適化手法）
        str->capacity = (new_len + 1) * 2;
        str->data = (char *)kmrealloc(str->data, str->capacity, strlen(str->data));
        if (str->data == NULL) {
            error("no memory for capacity %d", str->capacity);
            return -1;
        }
    }

    // 文字列を結合し、長さを更新
    if (str->data) {
        safestrcpy(str->data + str->length, value, append_len);
        str->length = new_len;
    }

    return 0;
}

int cstring_replace(cstring_t *str, const char *find, const char *replace)
{
    if (str_replace(&str->data, find, replace) < 0)
        return -1;
    str->length = str->capacity = strlen(str->data);
    return 0;
}

int cstring_truncate(cstring_t *str)
{
    memset(str->data, 0, str->length);
    str->length = 0;
    return 0;
}

// メモリ解放関数
void cstring_free(cstring_t *str) {
    kmfree(str->data);
    str->data = NULL;
    str->length = 0;
    str->capacity = 0;
}
