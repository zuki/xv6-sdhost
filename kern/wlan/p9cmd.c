#include <types.h>
#include <wlan/p9cmd.h>
#include <wlan/p9error.h>
#include <string.h>
#include <console.h>

/* コマンドエラーをprintする */
void cmderror(Cmdbuf *cb, const char *err)
{
    if (cb->argc > 0) {
        char Cmd[256];
        int p = 0;
        for (unsigned i = 0; i < cb->argc; i++) {
            if (i > 0)
                Cmd[p++]= ' ';
            memmove(Cmd + p, cb->f[i], strlen(cb->f[i]));
            p += strlen(cb->f[i]);
        }
        assert(p < 256);
        Cmd[p] = '\0';
        print("%s\n", Cmd);
    }
    p9error(err);
}

/* コマンドをパースしてCmdbufにセットして返す */
Cmdbuf *parsecmd(const void *str, long n)
{
    Cmdbuf *pCmdbuf = (Cmdbuf *) kmalloc(sizeof(Cmdbuf));
    assert(pCmdbuf != 0);

    /* コマンド文字列をバッファにコピー */
    strncpy(pCmdbuf->buf, (const char *)str, sizeof pCmdbuf->buf-1);
    pCmdbuf->buf[sizeof pCmdbuf->buf-1] = '\0';

    pCmdbuf->argc = 0;

    /* " \t\n"区切りでコマンドと引数を取り出しf[]にセットする */
    char *pSavePtr;
    for (unsigned i = 0; i < nelem (pCmdbuf->f); i++) {
        char *p = strtok_r(i == 0 ? pCmdbuf->buf : NULL, " \t\n", &pSavePtr);
        pCmdbuf->f[i] = p;
        if (p == 0) {
            break;
        }

        char *Arg = kmalloc(strlen(p)+1);
        strncpy(Arg, p, strlen(p));
        if (str_replace(&Arg, "\\x20", " ") > 0) {
            strncpy(p, Arg, strlen(Arg));
        }
        kmfree(Arg);
        pCmdbuf->argc++;
    }

    return pCmdbuf;
}

/* ct[nelem]からコマンドバッファcbのコマンドと同じものを探し、見つかったらそのctを */
/* 見つからなかった場合はエラー表示をして0を返す */
Cmdtab *lookupcmd (Cmdbuf *cb, Cmdtab *ct, size_t nelem)
{
    if (cb->argc == 0) {
        cmderror(cb, "Command expected");
    }

    for (unsigned i = 0; i < nelem; i++, ct++) {
        if (strcasecmp(cb->f[0], ct->cmd) == 0) {
            return ct;
        }
    }

    cmderror(cb, "Invalid command");

    return 0;
}
