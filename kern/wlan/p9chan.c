#include <types.h>
#include <wlan/p9chan.h>
#include <wlan/p9error.h>
#include <mm.h>
#include <string.h>
#include <param.h>

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
    // Path: ファームウェアファイルの完全名 : "/D/firmware/name" -> "4:/firmware/name"に変換
    char Path[strlen(s_pPath)+strlen(name)+1+1];  // '/' + '\0' ; /d/ ->4:/
    sprintf(Path, "%d:%s/%s", SECONDDEV, (const char *) s_pPath+2, name);
    if(0)print("dir: %s, file: %s, path: %s\n", s_pPath, name, Path);
    // ファームウェアファイルをオープンする
    FRESULT Result = f_open(&c->file, Path, FA_READ | FA_OPEN_EXISTING);
    if (Result != FR_OK) {
        kmfree(c);
        print("Could not open %s for %d\n", Path, Result);
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
    print("cclose");
    assert (c->open);
    f_close(&c->file);
    c->open = 0;
    kmfree(c);
    print(" ok\n");
}

// チャンネルcをオフセットoffsetから長さlenだけbufに読み込み、
// 読み込んだバイト数を返す
static int readchan(Chan *c, void *buf, size_t len, uint64_t offset)
{
    assert (c->open);

    FRESULT Result;
    if(offset == 0)print("readchan c->off: 0x%x, offset: 0x%x, f->fptr: 0x%x\n", c->offset, offset, c->file.fptr);
    if (c->offset != offset) {
        Result = f_lseek(&c->file, offset);
        assert (Result == FR_OK);
        c->offset = offset;
    }
    if(offset == 0)print("after seek c->off: 0x%x, f->fptr: 0x%x\n", c->offset, c->file.fptr);
    unsigned nBytesRead;
    Result = f_read(&c->file, buf, len, &nBytesRead);
    if (Result != FR_OK) {
        print("readchan f_read failed with %d\n", Result);
        p9error(Eio);
        return -1;
    }
    c->offset += nBytesRead;
    if(offset == 0)print("byte: %d, offset: 0x%x\n", nBytesRead, c->offset);
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
