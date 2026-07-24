#include <types.h>
#include <wlan/p9chan.h>
#include <wlan/p9error.h>
#include <mm.h>
#include <string.h>
#include <param.h>
#include <fs/fatfs/ff.h>
#include <proc.h>

static char *firmpath = 0;

// ファームウェアディレクトリのファイル名 nameのファームウェアファイルを
// チャンネルとして開く
Chan *namec(const char *name, unsigned func, unsigned flags, unsigned opt)
{
    Chan *c = kmalloc(sizeof(Chan));
    assert (c != 0);

    c->type = 0;
    c->open = 0;

    assert (firmpath != 0);
    // Path: ファームウェアファイルの完全名 : "/D/firmware/name" -> "4:/firmware/name"に変換
    //    ここではfirmwareのディレクトリを最初からドライバ番号で指定することにする(proc.c) 4:/firmapath/name
    char Path[strlen(firmpath)+strlen(name)+1+1];  // '/' + '\0' ; /d/ ->4:/
    //sprintf(Path, "%d:%s/%s", SECONDDEV, (const char *) firmpath+2, name);
    sprintf(Path, "%s/%s", (const char *) firmpath, name);
    if(0)print("dir: %s, file: %s, Path: %s\n", firmpath, name, Path);
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
    assert (c->open);
    f_close (&c->file);
    c->open = 0;
    kmfree(c);
    if(0)print(" ok\n");
}

// チャンネルcをオフセットoffsetから長さlenだけbufに読み込み、
// 読み込んだバイト数を返す
static int readchan(Chan *c, void *buf, size_t len, uint64_t offset)
{
    assert (c->open);
    FRESULT Result;

    if(0)print("readchan c->off: 0x%x, offset: 0x%x, f->fptr: 0x%x\n", c->offset, offset, c->file.fptr);
    if (c->offset != offset) {
        Result = f_lseek(&c->file, offset);
        assert (Result == FR_OK);
        c->offset = offset;
    }
    if(0)print("after seek c->off: 0x%x, f->fptr: 0x%x\n", c->offset, c->file.fptr);
    unsigned nBytesRead;
    Result = f_read(&c->file, buf, len, &nBytesRead);
    if (Result != FR_OK) {
        print("readchan f_read failed with %d\n", Result);
        p9error(Eio);
        return -1;
    }
    c->offset += nBytesRead;
    if(0)print("byte: %d, offset: 0x%x\n", nBytesRead, c->offset);
    return (int) nBytesRead;
}

// struct device_tの配列

struct device_t devchan = { readchan };

struct device_t *devtab[1] = {
    &devchan,
};

void p9chan_init(const char *path)
{
    assert (firmpath == 0);
    // ファームウェアが存在するパスをセット
    firmpath = path;
    assert (firmpath != 0);
}
