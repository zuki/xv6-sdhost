#ifndef H_INC_WLAN_P9SD_H
#define H_INC_WLAN_P9SD_H

#include <p9util.h>

// SDIO操作関数構造体
typedef struct sdio_t
{
    const char *devname;
    int (*init) (void);
    void (*enable) (void);
    int (*inquiry) (char *buf, int len);
    int (*cmd) (uint32_t cmd, uint32_t arg, uint32_t *resp);
    void (*iosetup) (int write, void *buf, int blksize, int blkcount);
    void (*io)(int write, uchar *buf, int len);
} SDio;

extern SDio sdio;

#endif
