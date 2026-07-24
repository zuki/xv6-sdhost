#ifndef H_INC_WLAN_P9CHAN_H
#define H_INC_WLAN_P9CHAN_H

#include <fs/fatfs/ff.h>
#include <p9util.h>

// チャンネル構造体
typedef struct {
    unsigned type;      // ファイル種別
    int open;           // 1: open済み
    FIL file;           // ファイル
    ulong offset;       // ファイルオフセット
} Chan;

Chan *namec(const char *name, unsigned func, unsigned flags, unsigned opt);
#define Aopen        0
#define OREAD        0

void cclose(Chan *chan);

#define devtab    __p9devtab
// チャンネルの操作関数構造体
struct device_t
{
    int (*read) (Chan *chan, void *buf, size_t len, uint64_t off);
};

extern struct device_t *devtab[];

void p9chan_init (const char *path);

#endif
