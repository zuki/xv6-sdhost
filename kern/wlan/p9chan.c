#include <types.h>
#include <wlan/p9chan.h>
#include <wlan/p9error.h>
#include <mm.h>
#include <string.h>

static char *s_pPath = 0;

// ファームウェアディレクトリのファイル名 nameのファームウェアファイルを
// チャンネルとして開く
Chan *namec(const char *name, unsigned func, unsigned flags, unsigned opt)
{
    Chan *c = kmalloc(sizeof(Chan));
    assert (c != 0);

    c->type = 0;
    c->open = 0;

    assert (s_pPath != 0);
    // Path: ファームウェアファイルの完全名 : "/D/firmware/name"
    char Path[strlen(s_pPath)+strlen(name)+1];
    sprintf(Path, "%s%s", (const char *) *s_pPath, name);

    // ファームウェアファイルをオープンする
    FRESULT Result = f_open(&c->file, Path, FA_READ | FA_OPEN_EXISTING);
    if (Result != FR_OK) {
        kmfree(c);
        print("File: %s", Path);
        p9error(Enonexist);
        return 0;
    }

    c->open = 1;
    c->offset = 0;

    return c;
}

// チャンネルを閉じる
void cclose(Chan *c)
{
    assert (c->open);
    f_close(&c->file);
    c->open = 0;
    kmfree(c);
}

// チャンネルcをオフセットoffsetから長さlenだけbufに読み込み、
// 読み込んだバイト数を返す
static int readchan(Chan *c, void *buf, size_t len, uint64_t offset)
{
    assert (c->open);

    FRESULT Result;
    if (c->offset != offset) {
        Result = f_lseek(&c->file, offset);
        assert (Result == FR_OK);
        c->offset = offset;
    }

    unsigned nBytesRead;
    Result = f_read(&c->file, buf, len, &nBytesRead);
    if (Result != FR_OK) {
        p9error(Eio);
        return -1;
    }
    c->offset += nBytesRead;
    return (int) nBytesRead;
}

// struct device_tの配列

struct device_t devchan = { readchan };

struct device_t *devtab[1] = {
    &devchan,
};

void p9chan_init(const char *path)
{
    assert (s_pPath == 0);
    // ファームウェアが存在するパスをセット
    s_pPath = path;
    assert (s_pPath != 0);
}
