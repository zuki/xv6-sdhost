/*
 * Broadcom bcm4330 wifi (sdio interface)
 */
// https://9p.io/sources/contrib/miller/9/bcm/ether4330.c

#include <wlan/p9compat.h>
#include <spinlock.h>
#include <mm.h>
#include <string.h>
#include <proc.h>
#include <string.h>

extern int sdiocardintr(int);

#define CACHELINESZ 64    /* temp */

/*  各種定数の定義 */
enum{
    SDIODEBUG   = 0,        /* SDIOのデバッグ*/
    SBDEBUG     = 0,        /* Silicon Backplaneのデバッグ */
    EVENTDEBUG  = 0,        /* イベントのデバッグ*/
    VARDEBUG    = 0,        /* 変数のデバッグ*/
    FWDEBUG     = 0,        /* ファームウェアのデバッグ*/

    Corescansz  = 512,      /* コアのスキャンサイズ*/
    Uploadsz    = 2048,     /* アップロードサイズ*/

    Wifichan    = 0,        /* デフォルトチャンネル */
    Firmwarecmp = 1,        /* ファームウェアのロードチェックを行う */

    ARMcm3      = 0x82A,    /* CYW43455の内蔵CPUはこれ */
    ARM7tdmi    = 0x825,
    ARMcr4      = 0x83E,

    Fn0     = 0,            /* 機能 0 : SDIO制御に仕様 */
    Fn1     = 1,            /* 機能 1 : チップ内レジスタ、メモリとの読み書きに使用 */
    Fn2     = 2,            /* 機能 2 : WiFiパケットの読み書きに使用 */
    Fbr1    = 0x100,        /* FBR 1の基底アドレス  */
    Fbr2    = 0x200,        /* FBR 2の基底アドレス  */

    /* CCCR レジスタアドレス */
    Ioenable    = 0x02,     /* I/O Enable */
    Ioready     = 0x03,     /* I/O Ready */
    Intenable   = 0x04,     /* Int Enable */
    Intpend     = 0x05,     /* Int Pending */
    Ioabort     = 0x06,     /* I/O Abort */
    Busifc      = 0x07,     /* Bus Interface Control */
    Capability  = 0x08,     /* Card Capability */
    Blksize     = 0x10,     /* FN0 Block Size */
    Highspeed   = 0x13,     /* Bus Speed Select */

    /* SDIO コマンド */
    GO_IDLE_STATE       = 0,    /* CMD0 */
    SEND_RELATIVE_ADDR  = 3,    /* CMD3 */
    IO_SEND_OP_COND     = 5,    /* CMD5 */
    SELECT_CARD         = 7,    /* CMD7 */
    VOLTAGE_SWITCH      = 11,   /* CMD11: オプション */
    IO_RW_DIRECT        = 52,   /* CMD52 */
    IO_RW_EXTENDED      = 53,   /* CMD53 */

    /* SELECT_CARD (CMD7) の引数 */
    Rcashift    = 16,       /* 引数内のRCAビット位置 */

    /* SEND_OP_COND (CMD5) の引数 */
    Hcs     = 1<<30,    /* (ACMD41の引数） host supports SDHC & SDXC */
    V3_3    = 3<<20,    /* 3.2-3.4 volts */
    V2_8    = 3<<15,    /* 2.7-2.9 volts */
    V2_0    = 1<<8,     /* 2.0-2.1 volts */
    S18R    = 1<<24,    /* S18R: switch to 1.8V request */

    /* Sonic's Silicon Backplane (CYW43**上のコアへのアクセス) */
    /*   src/bus_protocols/whd_sdio.h */
    Sbwsize = 0x8000,
    Sb32bit = 0x8000,
    Sbaddr  = 0x1000a,  /* backplane address LOW, b=MIDDLE, c=HIGH */
        Enumbase    = 0x18000000,
    Framectl= 0x1000d,
        Rfhalt      = 0x01,     /* Read Frame */
        Wfhalt      = 0x02,     /* Write Frame */
    Clkcsr  = 0x1000e,
        ForceALP    = 0x01,     /* active low-power clock */
        ForceHT     = 0x02,     /* high throughput clock */
        ForceILP    = 0x04,     /* idle low-power clock */
        ReqALP      = 0x08,
        ReqHT       = 0x10,
        Nohwreq     = 0x20,
        ALPavail    = 0x40,
        HTavail     = 0x80,
    Pullups = 0x1000f,
    Wfrmcnt = 0x10019,
    Rfrmcnt = 0x1001b,

    /* chip core 制御レジスタ src/whd_chip.c */
    Ioctrl      = 0x408,
    Resetctrl   = 0x800,

    /* SoCRAMレジスタ */
    Coreinfo    = 0x00,
    Bankidx     = 0x10,
    Bankinfo    = 0x40,
    Bankpda     = 0x44,

    /* armcr4 (arm Cortex-R4) レジスタ */
    Cr4Cap      = 0x04,
    Cr4Bankidx  = 0x40,
    Cr4Bankinfo = 0x44,
    Cr4Cpuhalt  = 0x20,

    /* チップ共通レジスタ */
    Gpiopullup  = 0x58,
    Gpiopulldown= 0x5c,
    Chipctladdr = 0x650,
    Chipctldata = 0x654,

    /* sdio core regs */
    Intstatus   = 0x20,         /* 割り込みステータス */
        Fcstate     = 1<<4,
        Fcchange    = 1<<5,
        FrameInt    = 1<<6,
        MailboxInt  = 1<<7,
    Intmask     = 0x24,         /* 割り込みマスク */
    Sbmbox      = 0x40,         /* SB mbox */
    Sbmboxdata  = 0x48,         /* Sb mbox data */
    Hostmboxdata= 0x4c,         /* Host mbox data */
        Fwready     = 0x80,

    /* wifi control commands */
    GetVar      = 262,
    SetVar      = 263,

    /* status */
    Disconnected= 0,
    Connecting,
    Connected,
};

typedef struct Ctlr Ctlr;

/*  パスワードに関係する定数 */
enum{
    Wpa         = 1,
    Wep         = 2,
    Wpa2        = 3,
    WNameLen    = 32,
    WNKeys      = 4,
    WKeyLen     = 32,
    WMinKeyLen  = 5,
    WMaxKeyLen  = 13,
};

typedef struct WKey WKey;
/*  WiFiパスワード構造体 */
struct WKey
{
    ushort    len;
    char    dat[WKeyLen];
};

/*  WiFiチップ制御構造体 */
struct Ctlr {
    Ether*  edev;                   /* ethernet デバイス構造体へのポインタ */
    QLock   cmdlock;                /* コマンドロック */
    QLock   pktlock;                /* パケットロック */
    QLock   tlock;                  /* FIXME: timeoutロック */
    QLock   alock;
    struct spinlock txwinlock;      /* 送信ウィンドウロック */
    Rendez  cmdr;                   /* sleep時の待機オブジェクト */
    Rendez  joinr;                  /* sleep時の待機オブジェクト */
    int     joinstatus;             /* 接続状態 */
    int     cryptotype;             /* 暗号種別 */
    int     chanid;                 /* チャネルID */
    uchar   bssid[Eaddrlen];        /* SSIDの識別子: APのMACアドレス */
    char    essid[WNameLen + 1];    /* WiFiネットワーク名 */
    WKey    keys[WNKeys];           /* ネットワーク接続キーワード */
    Block   *rsp;                   /* 応答ブロック */
    Block   *scanb;                 /* スキャン結果ブロック */
    int     scansecs;               /* スキャン時間 */
    int     status;                 /* ステータス */
    int     chipid;                 /* チップID */
    int     chiprev;                /* チップリビジョン */
    int     armcore;                /* armcore種別 */
    char    *regufile;              /* 規制データファイル .clm_blob */
    union {
        uint32_t  i;
        uchar   c[4];
    } resetvec;                     /* リセットベクタ */
    ulong   chipcommon;             /* 共通ベースアドレス */
    ulong   armctl;                 /* ARM ctl */
    ulong   armregs;                /* ARMレジスタ */
    ulong   d11ctl;
    ulong   socramregs;             /* SoC RAM レジスタ */
    ulong   socramctl;              /* SoC RAM ctl */
    ulong   sdregs;                 /* SD レジスタ */
    int     sdiorev;                /* SDIO リビジョン */
    int     socramrev;              /* SoC RAM リビジョン */
    ulong   socramsize;             /* SoC RAM サイズ */
    ulong   rambase;                /* SoC RAM 基底アドレス */
    short   reqid;                  /* リクエストID */
    uchar   fcmask;                 /* FC マスク */
    uchar   txwindow;               /* 送信ウィンドウ */
    uchar   txseq;                  /* 送信シーケンス番号 */
    uchar   rxseq;                  /* 受信シーケンス番号 */
    ether_event_handler_t *evhndlr; /* イベントハンドラ関数へのポインタ */
    void    *evcontext;             /* イベントコンテキスト */
};

/*  CTLRコマンド */
enum{
    CMauth,
    CMchannel,
    CMcrypt,
    CMessid,
    CMkey1,
    CMkey2,
    CMkey3,
    CMkey4,
    CMrxkey,
    CMrxkey0,
    CMrxkey1,
    CMrxkey2,
    CMrxkey3,
    CMtxkey,
    CMdebug,
    CMjoin,
    CMdisassoc,
    CMescan,
    CMcountry,
    CMcreate,
    CMdown,
};

/*  CTLR コマンド情報 : { コマンド番号、コマンド名、コマンド+引数の数 } */
static Cmdtab cmds[] = {
    {CMauth,        "auth",     2},
    {CMchannel,     "channel",  2},
    {CMcrypt,       "crypt",    2},
    {CMessid,       "essid",    2},
    {CMkey1,        "key1",     2},
    {CMkey2,        "key2",     2},
    {CMkey3,        "key3",     2},
    {CMkey4,        "key4",     2},
    {CMrxkey,       "rxkey",    3},
    {CMrxkey0,      "rxkey0",   3},
    {CMrxkey1,      "rxkey1",   3},
    {CMrxkey2,      "rxkey2",   3},
    {CMrxkey3,      "rxkey3",   3 },
    {CMtxkey,       "txkey",    3},
    {CMdebug,       "debug",    2},
    {CMjoin,        "join",     5},
    {CMdisassoc,    "disassoc", 2},
    {CMescan,       "escan",    2},
    {CMcountry,     "country",  2},
    {CMcreate,      "create",   4},
    {CMdown,        "down",     1},
};

typedef struct Sdpcm Sdpcm;
typedef struct Cmd Cmd;
/*  SDPCM: bus layer for SDIO  の ヘッダー構造体 (12バイト) */
/*  https://infineon.github.io/wifi-host-driver/html/index.html */
struct Sdpcm {
    uchar    len[2];        /* SDPCM packet size */
    uchar    lenck[2];
    uchar    seq;           /* Sequence number of pkt */
    uchar    chanflg;       /* channel_and_flags : IOCTL/IOVAR or User Data or Event */
    uchar    nextlen;
    uchar    doffset;       /* header_length : Offset to BDC or CDC header */
    uchar    fcmask;        /* wireless_flow_control */
    uchar    window;        /* bus_data_credit */
    uchar    version;
    uchar    pad;
};

/*  CDCヘッダー構造体 (16バイト) */
struct Cmd {
    uchar    cmd[4];        /* ioctl command value */
    uchar    len[4];        /* lower 16: output buflen; upper 16: input buflen (excludes header)*/
    uchar    flags[2];      /* flag defns given in bcmcdc.h */
    uchar    id[2];
    uchar    status[4];     /* status code returned from the device */
};

static char config40181[] = "bcmdhd.cal.40181";
static char config40183[] = "bcmdhd.cal.40183.26MHz";

/*  ファームウェアファイル構造体 */
/*  regufileについては https://community.infineon.com/t5/%E3%83%8A%E3%83%AC%E3%83%83%E3%82%B8%E3%83%99%E3%83%BC%E3%82%B9%E3%82%A2%E3%83%BC%E3%83%86%E3%82%A3%E3%82%AF%E3%83%AB-KBA/CLM-BLOB%E3%81%8B%E3%82%89C%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%B8%E3%81%AE%E5%A4%89%E6%8F%9B/ta-p/640360 を参照 */
static struct {
    int chipid;         /* チップID */
    int chiprev;        /* チップリビジョン */
    char *fwfile;       /* ファームウェアファイル名 */
    char *cfgfile;      /* コンフィグレーションファイル名 */
    char *regufile;     /* 各地域の規制データファイル名 */
} firmware[] = {
    { 0x4330, 3,    "fw_bcm40183b1.bin", config40183, 0 },
    { 0x4330, 4,    "fw_bcm40183b2.bin", config40183, 0 },
    { 43362,  0,    "fw_bcm40181a0.bin", config40181, 0 },
    { 43362,  1,    "fw_bcm40181a2.bin", config40181, 0 },
    { 43430,  1,    "brcmfmac43430-sdio.bin", "brcmfmac43430-sdio.txt", "brcmfmac43430-sdio.clm_blob" },
    { 43430,  2,    "brcmfmac43436-sdio.bin", "brcmfmac43436-sdio.txt", "brcmfmac43436-sdio.clm_blob" },
    /*  This may be necessary for newer Raspberry Pi Zero 2 W: */
    // { ???, ???,    "brcmfmac43436s-sdio.bin", "brcmfmac43436s-sdio.txt", 0 },
    /*  Raspi3B+は次を使う */
    { 0x4345, 6, "brcmfmac43455-sdio.bin", "brcmfmac43455-sdio.txt", "brcmfmac43455-sdio.clm_blob" },
    { 0x4345, 9, "brcmfmac43456-sdio.bin", "brcmfmac43456-sdio.txt", "brcmfmac43456-sdio.clm_blob" },
};

static QLock sdiolock;
static int iodebug = 0;

static void bcmevent(Ctlr*, uchar*, int);
static void wlscanresult(Ether*, uchar*, int);
static void wlsetvar(Ctlr*, char*, void*, int);
static void etherbcmscan(void *a, uint secs);
static void callevhndlr(Ctlr*, ether_event_type_t, const ether_event_params_t *);

/*  リトルエンディアンの2バイトのvをビッグエンディアンに変換してバッファｐに */
/*  セットし、セット後のpの位置を返す */
static uchar * put2(uchar *p, short v)
{
    p[0] = v;
    p[1] = v >> 8;
    return p + 2;
}

/*  リトルエンディアンの4バイトvをビッグエンディアンに変換してバッファｐに */
/*  セットし、セット後のpの位置を返す */
static uchar * put4(uchar *p, long v)
{
    p[0] = v;
    p[1] = v >> 8;
    p[2] = v >> 16;
    p[3] = v >> 24;
    return p + 4;
}

/*  ビッグエンディアンの4バイトをリトルエンディアンに変換して返す */
static ulong get4(uchar *p)
{
    return p[0] | p[1]<<8 | p[2]<<16 | p[3]<<24;
}

static void dump(char *s, void *a, int n)
{
    hexdump (a, n, s);
}

/*
 * (1) EMMC (host) と CYW4335 (client) 間をSDIOで通信する
 */
/*  コマンドcmdを実行して（sdiolockは呼び出し側でロックされている）応答を返す */
static ulong sdiocmd_locked(int cmd, ulong arg)
{
    uint32_t resp[4];

    sdio.cmd(cmd, arg, resp);
    return resp[0];
}

/*  コマンドcmdを実行して（sdiolockは関数内でロックする）応答を返す */
static ulong sdiocmd(int cmd, ulong arg)
{
    ulong r;

    qlock(&sdiolock);
    if (waserror()) {
        if (SDIODEBUG) print("sdiocmd error: cmd %d arg %lx\n", cmd, arg);
        qunlock(&sdiolock);
        nexterror();
    }

    r = sdiocmd_locked(cmd, arg);
    qunlock(&sdiolock);
    poperror();
    return r;

}

/*  コマンドcmdを実行して（sdiolockは呼び出し先の関数でロックする）応答を返す */
static ulong trysdiocmd(int cmd, ulong arg)
{
    ulong r;

    if (waserror())
        return 0;
    r = sdiocmd(cmd, arg);
    poperror();
    return r;
}

/*  CMD52で機能fnのアドレスaddrのデータ(8bit)を読み取る */
static int sdiord(int fn, int addr)
{
    int r;
                            /*  Read | funct number | address */
    r = sdiocmd(IO_RW_DIRECT, (0<<31)|((fn&7)<<28)|((addr&0x1FFFF)<<9));
    /*  r[15:8]: レスポンスフラグ, r[7:0]: R/W データ */
    /*  レスポンスフラグでエラーのチェック */
    if (r & 0xCF00) {
        print("ether4330: sdiord(%x, %x) fail: %02x %02x\n", fn, addr, (r>>8)&0xFF, r&0xFF);
        p9error(Eio);
    }
    return r & 0xFF;
}

/*  CMD52で8bitのデータdataを機能fnのアドレスaddrに書き出す */
/*   エラーが発生した場合は最大10回再試行する */
static void sdiowr(int fn, int addr, int data)
{
    int r;
    int retry;

    r = 0;
    for (retry = 0; retry < 10; retry++) {
        r = sdiocmd(IO_RW_DIRECT, (1<<31)|((fn&7)<<28)|((addr&0x1FFFF)<<9)|(data&0xFF));
        /*  書き出し成功 */
        if ((r & 0xCF00) == 0)
            return;
    }
    /*  10回試行してもエラー */
    print("ether4330: sdiowr(%x, %x, %x) fail: %02x %02x\n", fn, addr, data, (r>>8)&0xFF, r&0xFF);
    p9error(Eio);
}

/*  CMD53で長さlenのバッファaのデータを機能fnのアドレスaddrに書き出す */
/*  incr=1の場合は1ブロック書き出す毎にaddrを増分する */
static void sdiorwext(int fn, int write, void *a, int len, int addr, int incr)
{
    int bsize, blk, bcount, m;

    /*  ブロックサイズは機能2とそれ以外で異なる */
    bsize = fn == Fn2? 512 : 64;
    /*  CMD53を実行する */
    while (len > 0) {
        /*  ブロック数の最大値は511 */
        if (len >= 511*bsize) {
            blk = 1;
            bcount = 511;
            m = bcount*bsize;
        } else if (len > bsize) {
            blk = 1;
            bcount = len/bsize;
            m = bcount*bsize;
        /*  ブロックサイズよりlenが小さい場合はバイト転送にする */
        } else {
            blk = 0;
            bcount = len;
            m = bcount;
        }
        qlock(&sdiolock);
        if (waserror()) {
            print("ether4330: sdiorwext fail: %s\n", up->errstr);
            qunlock(&sdiolock);
            nexterror();
        }
        /*  ブロックサイズをセットする（EMMCのBLKSIZECNTにセット） */
        /*  FIXME: SDIOの仕様ではブロックサイズはCCCR(FN0), FBR(FN1-7)の */
        /*         ブロックサイズレジスタに書き込むとあるが？ */
        if (blk)
            sdio.iosetup(write, a, bsize, bcount);
        else
            sdio.iosetup(write, a, bcount, 1);
        /*  CMD53を送信 */
        sdiocmd_locked(IO_RW_EXTENDED,
            write<<31 | (fn&7)<<28 | blk<<27 | incr<<26 | (addr&0x1FFFF)<<9 | (bcount&0x1FF));
        /*  データを送受信 */
        sdio.io(write, a, m);
        qunlock(&sdiolock);
        poperror();
        len -= m;
        a = (char*)a + m;
        if (incr)
            addr += m;
    }
}

/*  (*addr | bits) をaddrに書き込む */
static void sdioset(int fn, int addr, int bits)
{
    sdiowr(fn, addr, sdiord(fn, addr) | bits);
}

/*  sdioを初期化する */
static void sdioinit(void)
{
    ulong ocr, rca;
    int i;

    /* SDカードをEMMCモジュールから切り離し、SDHOSTにつなげる */
    /* sdhost.c で設定 */
#if 0
    for (i = 48; i <= 53; i++)
        gpiosel(i, Alt0);
#endif

    /* EMMCモジュールをWiFiにつなげる */
    for (i = 34; i <= 39; i++) {
        gpiosel(i, Alt3);
        if (i == 34)
            gpiopulloff(i);
        else
            gpiopullup(i);
    }

    sdio.init();                            /* EMMCを初期化する */
    sdio.enable();                          /* EMMCを有効化する */
    sdiocmd(GO_IDLE_STATE, 0);              /* CMD0: SDモードにする */
    ocr = trysdiocmd(IO_SEND_OP_COND, 0);   /* CMD5: OCRを取得 */
    i = 0;
    /*  3.2-3.4Vで電源オン */
    while ((ocr & (1<<31)) == 0) {
        if (++i > 5) {
            print("ether4330: no response to sdio access: ocr = %lx\n", ocr);
            p9error(Eio);
        }
        ocr = trysdiocmd(IO_SEND_OP_COND, V3_3);    /* CMD5: WV=3.2-3.4v*/
        p9tsleep(&up->sleep, return0, nil, 100);
    }
    rca = sdiocmd(SEND_RELATIVE_ADDR, 0) >> Rcashift;   /* CMD3: RCAを取得 */
    sdiocmd(SELECT_CARD, rca << Rcashift);              /* CMD7: RCAで選択 */
    sdioset(Fn0, Highspeed, 2);                         /* CCCR: 13h -> 25MHx */
    sdioset(Fn0, Busifc, 2);                            /* CCCR: 07h -> バス幅 4 */
    sdiowr(Fn0, Fbr1+Blksize, 64);                      /* FBR1: 110h-111h */
    sdiowr(Fn0, Fbr1+Blksize+1, 64>>8);                 /*  Fn1のブロックサイズ 64 */
    sdiowr(Fn0, Fbr2+Blksize, 512);                     /* FBR2: 210h-211h */
    sdiowr(Fn0, Fbr2+Blksize+1, 512>>8);                /*  Fn2のブロックサイズ 512 */
    sdioset(Fn0, Ioenable, 1<<Fn1);                     /* CCCR: 02h -> Fn1をenable */
    sdiowr(Fn0, Intenable, 0);                          /* CCCR: 04h -> 割り込みdisable */
    for (i = 0; !(sdiord(Fn0, Ioready) & 1<<Fn1); i++) { /* CCCR: 03h -> Fn1がenableになるのを待つ */
        if (i == 10) {
            print("ether4330: can't enable SDIO function\n");
            p9error(Eio);
        }
        p9tsleep(&up->sleep, return0, nil, 100);
    }
}

/*  SDIOカードをリセット : SDIO仕様 4.2.2参照 */
static void sdioreset(void)
{
    sdiowr(Fn0, Ioabort, 1<<3);    /* CCCR: 06h -> リセット */
}

/*  FNをアボートさせる : SDIO仕様 CCCR: 06h: ASxを参照 */
static void sdioabort(int fn)
{
    sdiowr(Fn0, Ioabort, fn);
}

/*
 * (2) EMMC (host) と CYW4335 の間でSDIOのFn1(チップのレジスタとメモリ)と
 * Fn2（Ethernetパケット）経由で通信する
 */

/*  CMD52: Fn1のアドレスオフセットoffにvalを書き込む */
static void
cfgw(ulong off, int val)
{
    sdiowr(Fn1, off, val);
}

/*  CMD52: Fn1のアドレスオフセットoffの値を読み込む */
static int cfgr(ulong off)
{
    return sdiord(Fn1, off);
}

/*  CMD53: 機能Fnのアドレスオフセットoffから４バイト読み込む */
static ulong cfgreadl(int fn, ulong off)
{
    uchar cbuf[2*CACHELINESZ];
    uchar *p;

    p = (uchar*)ROUND((uintptr_t)cbuf, CACHELINESZ);
    memset(p, 0, 4);
    sdiorwext(fn, 0, p, 4, off|Sb32bit, 1);
    if (SDIODEBUG) print("cfgreadl %lx: %02x %02x %02x %02x\n", off, p[0], p[1], p[2], p[3]);
    return p[0] | p[1]<<8 | p[2]<<16 | p[3]<<24;
}

/*  CMD53: 機能Fnのアドレスオフセットoffに４バイトのデータdataを書き込む */
static void cfgwritel(int fn, ulong off, uint32_t data)
{
    uchar cbuf[2*CACHELINESZ];
    uchar *p;
    int retry;

    p = (uchar*)ROUND((uintptr_t)cbuf, CACHELINESZ);
    put4(p, data);
    if (SDIODEBUG) print("cfgwritel %lx: %02x %02x %02x %02x\n", off, p[0], p[1], p[2], p[3]);
    retry = 0;
    /*  機能fnをabort */
    while (waserror()) {
        print("ether4330: cfgwritel retry %lx %x\n", off, data);
        sdioabort(fn);
        if (++retry == 3)
            nexterror();
    }
    /*  機能fnに書き込む */
    sdiorwext(fn, 1, p, 4, off|Sb32bit, 1);
    poperror();
}

/*  チップ内部のアドレス用の基底アドレスをsbaddr[2-0]にセット */
/*    addr = 0x1234    SBaddr = 0x00, +1 = 0x00, +2 = 0x00 */
/*    addr = 0x8123           = 0x80,    = 0x00,      0x00 */
/*    addr = 0x123456         = 0x00,    = 0x12,      0x00 */
/*    addr = 0x12345678       = 0x00,    = 0x34,      0x12 */
static void sbwindow(ulong addr)
{
    addr &= ~(Sbwsize-1);
    cfgw(Sbaddr, addr>>8);
    cfgw(Sbaddr+1, addr>>16);
    cfgw(Sbaddr+2, addr>>24);
}

/*  Fn1を使った読み書き: アドレスはバイトアクセス */
static void sbrw(int fn, int write, uchar *buf, int len, ulong off)
{
    int n;
    USED(fn);

    if (waserror()) {
        print("ether4330: sbrw err off %lx len %d\n", off, len);
        nexterror();
    }
    if (write) {
        /*  ４バイト以上の場合、まず 4の倍数バイト分をCMD53でブロック転送する */
        if (len >= 4) {
            n = len;
            n &= ~3;
            sdiorwext(Fn1, write, buf, n, off|Sb32bit, 1);
            off += n;
            buf += n;
            len -= n;
        }
        /*  残りは１バイトずつ書き出す */
        while (len > 0) {
            sdiowr(Fn1, off|Sb32bit, *buf);
            off++;
            buf++;
            len--;
        }
    } else {
        if (len >= 4) {
            n = len;
            n &= ~3;
            sdiorwext(Fn1, write, buf, n, off|Sb32bit, 1);
            off += n;
            buf += n;
            len -= n;
        }
        while (len > 0) {
            *buf = sdiord(Fn1, off|Sb32bit);
            off++;
            buf++;
            len--;
        }
    }
    poperror();
}

/*  Fn1を使った読み書き: アドレスはウィンドウアドレス相対の32ビットワードアクセス */
static void sbmem(int write, uchar *buf, int len, ulong off)
{
    ulong n;

    n = ROUNDUP(off, Sbwsize) - off;
    if (n == 0)
        n = Sbwsize;
    if(0)print("sbmen buf:: 0x%p, len: 0x%x, off: 0x%x, n: %d\n", buf, len, off, n);
    while (len > 0) {
        if (n > len)
            n = len;
        sbwindow(off);
        if(0)print("n 0x%x off 0x%x buf 0x%p len 0x%x\n", n, off, buf, len);
        sbrw(Fn1, write, buf, n, off & (Sbwsize-1));
        off += n;
        buf += n;
        len -= n;
        n = Sbwsize;
    }
}

/* Fn2を使ったパケットの読み書き */
static void packetrw(int write, uchar *buf, int len)
{
    int n;
    int retry;

    n = 2048;
    while (len > 0) {
        if (n > len)
            n = ROUND(len, 4);
        retry = 0;
        while (waserror()) {
            sdioabort(Fn2);
            if (++retry == 3)
                nexterror();
        }
        sdiorwext(Fn2, write, buf, n, Enumbase, 0);
        poperror();
        buf += n;
        len -= n;
    }
}

/*
 * Silicon Backplane経由のチップコアの構成と制御
 */

/*  バックプレーンを無効にする */
static void sbdisable(ulong regs, int pre, int ioctl)
{
    sbwindow(regs);
    if ((cfgreadl(Fn1, regs + Resetctrl) & 1) != 0) {
        cfgwritel(Fn1, regs + Ioctrl, 3|ioctl);
        cfgreadl(Fn1, regs + Ioctrl);
        return;
    }
    cfgwritel(Fn1, regs + Ioctrl, 3|pre);
    cfgreadl(Fn1, regs + Ioctrl);
    cfgwritel(Fn1, regs + Resetctrl, 1);
    microdelay(10);
    while ((cfgreadl(Fn1, regs + Resetctrl) & 1) == 0)
        ;
    cfgwritel(Fn1, regs + Ioctrl, 3|ioctl);
    cfgreadl(Fn1, regs + Ioctrl);
}

/*  バックプレーンをリセットする */
static void sbreset(ulong regs, int pre, int ioctl)
{
    sbdisable(regs, pre, ioctl);
    sbwindow(regs);
    if (SBDEBUG) print("sbreset 0x%x 0x%lx 0x%lx ->", regs,
        cfgreadl(Fn1, regs+Ioctrl), cfgreadl(Fn1, regs+Resetctrl));
    while ((cfgreadl(Fn1, regs + Resetctrl) & 1) != 0) {
        cfgwritel(Fn1, regs + Resetctrl, 0);
        microdelay(40);
    }
    cfgwritel(Fn1, regs + Ioctrl, 1|ioctl);
    cfgreadl(Fn1, regs + Ioctrl);
    if (SBDEBUG) print("0x%lx 0x%lx\n",
        cfgreadl(Fn1, regs+Ioctrl), cfgreadl(Fn1, regs+Resetctrl));
}

/*  チップコアの情報を取得してctlオブジェクトにセット */
static void corescan(Ctlr *ctl, ulong r)
{
    uchar *buf;
    int i, coreid, corerev;
    ulong addr;

    buf = sdmalloc(Corescansz);
    if (buf == nil)
        p9error(Enomem);
    sbmem(0, buf, Corescansz, r);
    coreid = 0;
    corerev = 0;
    for (i = 0; i < Corescansz; i += 4) {
        switch (buf[i]&0xF) {
        case 0xF:    /* end */
            sdfree(buf);
            return;
        case 0x1:    /* コアの情報 : 0x01LLHHVV??01LLHHVV01 */
            if ((buf[i+4]&0xF) != 0x1)
                break;
            coreid = (buf[i+1] | buf[i+2]<<8) & 0xFFF;
            i += 4;
            corerev = buf[i+3];
            break;
        case 0x05:    /* アドレス : メモリ, 0x005LLMMHH, chip制御 0xc5LLMMHH */
            addr = buf[i+1]<<8 | buf[i+2]<<16 | buf[i+3]<<24;
            addr &= ~0xFFF;
            if (SBDEBUG) print("core %x %s 0x%p\n", coreid, buf[i]&0xC0? "ctl" : "mem", addr);
            switch(coreid) {
            case 0x800:    /* cpu */
                if ((buf[i] & 0xC0) == 0)
                    ctl->chipcommon = addr;
                break;
            case ARMcm3:
            case ARM7tdmi:
            case ARMcr4:
                ctl->armcore = coreid;
                if (buf[i] & 0xC0) {
                    if (ctl->armctl == 0)
                        ctl->armctl = addr;
                } else {
                    if (ctl->armregs == 0)
                        ctl->armregs = addr;
                }
                break;
            case 0x80E:    /* SoC */
                if (buf[i] & 0xC0)
                    ctl->socramctl = addr;
                else if (ctl->socramregs == 0)
                    ctl->socramregs = addr;
                ctl->socramrev = corerev;
                break;
            case 0x829:    /* SDIO */
                if ((buf[i] & 0xC0) == 0)
                    ctl->sdregs = addr;
                ctl->sdiorev = corerev;
                break;
            case 0x812:    /* D11 ? */
                if (buf[i] & 0xC0)
                    ctl->d11ctl = addr;
                break;
            }
        }
    }
    sdfree(buf);
}

/*  チップメモリの情報を取得してctlオブジェクトにセット */
static void ramscan(Ctlr *ctl)
{
    ulong r, n, size;
    int banks, i;

    if (ctl->armcore == ARMcr4) {
        r = ctl->armregs;
        sbwindow(r);
        n = cfgreadl(Fn1, r + Cr4Cap);
        if (SBDEBUG) print("cr4 banks %lx\n", n);
        banks = ((n>>4) & 0xF) + (n & 0xF);
        size = 0;
        for (i = 0; i < banks; i++) {
            cfgwritel(Fn1, r + Cr4Bankidx, i);
            n = cfgreadl(Fn1, r + Cr4Bankinfo);
            if (SBDEBUG) print("bank %d reg %lx size %ld\n", i, n, 8192 * ((n & 0x3F) + 1));
            size += 8192 * ((n & 0x3F) + 1);
        }
        ctl->socramsize = size;     /* RAMサイズ */
        ctl->rambase = 0x198000;    /* RAM基底アドレス */
        return;
    }
    if (ctl->socramrev <= 7 || ctl->socramrev == 12) {
        print("ether4330: SOCRAM rev %d not supported\n", ctl->socramrev);
        p9error(Eio);
    }
    /* ARMcr4以外のコアの場合 */
    sbreset(ctl->socramctl, 0, 0);
    r = ctl->socramregs;
    sbwindow(r);
    n = cfgreadl(Fn1, r + Coreinfo);
    if (SBDEBUG) print("socramrev %d coreinfo %lx\n", ctl->socramrev, n);
    banks = (n>>4) & 0xF;
    size = 0;
    for (i = 0; i < banks; i++) {
        cfgwritel(Fn1, r + Bankidx, i);
        n = cfgreadl(Fn1, r + Bankinfo);
        if (SBDEBUG) print("bank %d reg %lx size %ld\n", i, n, 8192 * ((n & 0x3F) + 1));
        size += 8192 * ((n & 0x3F) + 1);
    }
    ctl->socramsize = size;
    ctl->rambase = 0;
    if (ctl->chipid == 43430) {
        cfgwritel(Fn1, r + Bankidx, 3);
        cfgwritel(Fn1, r + Bankpda, 0);
    }
}

/* バックプレーンを初期化する */
static void
sbinit(Ctlr *ctl)
{
    ulong r;
    int chipid;
    char buf[16];

    sbwindow(Enumbase);
    r = cfgreadl(Fn1, Enumbase);
    chipid = r & 0xFFFF;
    sprint(buf, chipid > 43000 ? "%d" : "0x%x", chipid);
    /* ether4330: chip 0x4345 rev 6 type 1 */
    print("ether4330: chip %s rev %ld type %ld\n", buf, (r>>16)&0xF, (r>>28)&0xF);
    switch(chipid) {
        case 0x4330:
        case 43362:
        case 43430:
        case 0x4345:
            ctl->chipid = chipid;
            ctl->chiprev = (r>>16)&0xF;
            break;
        default:
            print("ether4330: chipid 0x%x (%d) not supported\n", chipid, chipid);
            p9error(Eio);
    }
    r = cfgreadl(Fn1, Enumbase + 63*4);
    corescan(ctl, r);
    if (ctl->armctl == 0 || ctl->d11ctl == 0 ||
       (ctl->armcore == ARMcm3 && (ctl->socramctl == 0 || ctl->socramregs == 0)))
        p9error("corescan didn't find essential cores\n");
    if (ctl->armcore == ARMcr4)
        sbreset(ctl->armctl, Cr4Cpuhalt, Cr4Cpuhalt);
    else
        sbdisable(ctl->armctl, 0, 0);
    sbreset(ctl->d11ctl, 8|4, 4);
    ramscan(ctl);
    /* ARM 0x18102000 D11 0x18101000 SOCRAM 0x0,0x0 819200 bytes @ 0x198000 */
    if (SBDEBUG) print("ARM 0x%p D11 0x%p SOCRAM 0x%p,0x%p %ld bytes @ 0x%p\n",
        ctl->armctl, ctl->d11ctl, ctl->socramctl, ctl->socramregs, ctl->socramsize, ctl->rambase);
    cfgw(Clkcsr, 0);
    microdelay(10);
    /* chipclk: 40 */
    if (SBDEBUG) print("chipclk: %x\n", cfgr(Clkcsr));
    cfgw(Clkcsr, Nohwreq | ReqALP);
    while ((cfgr(Clkcsr) & (HTavail|ALPavail)) == 0)
        microdelay(10);
    cfgw(Clkcsr, Nohwreq | ForceALP);
    microdelay(65);
    /* chipclk: 61 */
    if (SBDEBUG) print("chipclk: %x\n", cfgr(Clkcsr));
    cfgw(Pullups, 0);
    sbwindow(ctl->chipcommon);
    cfgwritel(Fn1, ctl->chipcommon + Gpiopullup, 0);
    cfgwritel(Fn1, ctl->chipcommon + Gpiopulldown, 0);
    if (ctl->chipid != 0x4330 && ctl->chipid != 43362)
        return;

    /* 以下は該当せず */
    cfgwritel(Fn1, ctl->chipcommon + Chipctladdr, 1);
    if (cfgreadl(Fn1, ctl->chipcommon + Chipctladdr) != 1)
        print("ether4330: can't set Chipctladdr\n");
    else{
        r = cfgreadl(Fn1, ctl->chipcommon + Chipctldata);
        if (SBDEBUG) print("chipcommon PMU (%lx) %lx", cfgreadl(Fn1, ctl->chipcommon + Chipctladdr), r);
        /* set SDIO drive strength >= 6mA */
        r &= ~0x3800;
        if (ctl->chipid == 0x4330)
            r |= 3<<11;
        else
            r |= 7<<11;
        cfgwritel(Fn1, ctl->chipcommon + Chipctldata, r);
        if (SBDEBUG) print("-> %lx (= %lx)\n", r, cfgreadl(Fn1, ctl->chipcommon + Chipctldata));
    }
}

/* バックプレーンをenableにする */
static void sbenable(Ctlr *ctl)
{
    int i;

    if (SBDEBUG) print("enabling HT clock...");
    cfgw(Clkcsr, 0);
    delay(1);
    /* HTクロックを要求 */
    cfgw(Clkcsr, ReqHT);
    /* THになるのを待つ : timeout = 5,000 ms */
    for (i = 0; (cfgr(Clkcsr) & HTavail) == 0; i++) {
        if (i == 50) {
            print("ether4330: can't enable HT clock: csr %x\n", cfgr(Clkcsr));
            p9error(Eio);
        }
        p9tsleep(&up->sleep, return0, nil, 100);
    }
    cfgw(Clkcsr, cfgr(Clkcsr) | ForceHT);
    delay(10);
    /* chipclk: d2 */
    if (SBDEBUG) print("chipclk: %x\n", cfgr(Clkcsr));
    sbwindow(ctl->sdregs);
    cfgwritel(Fn1, ctl->sdregs + Sbmboxdata, 4 << 16);      /* protocol version */
    cfgwritel(Fn1, ctl->sdregs + Intmask, FrameInt | MailboxInt | Fcchange);    /* 割り込みマスク */
    sdioset(Fn0, Ioenable, 1<<Fn2);                         /* Fn2のIOをenable */
    for (i = 0; !(sdiord(Fn0, Ioready) & 1<<Fn2); i++) {    /* Fn2のIOがenableになるのを待機 */
        if (i == 10) {
            print("ether4330: can't enable SDIO function 2 - ioready %x\n", sdiord(Fn0, Ioready));
            p9error(Eio);
        }
        p9tsleep(&up->sleep, return0, nil, 100);
    }
    sdiowr(Fn0, Intenable, (1<<Fn1) | (1<<Fn2) | 1);    /* Fn0, Fn1, Fn2の割り込みを有効に */
}

/*
 * ファームウェアとconfigファイルのアップロード
 */

/*
 * configファイルを'var=value\0'のリストの形に処理する (in buffer buf with length n):
 *    - （'#'で始まる）コメントと空白行を削除
 *    - CR (0x0d)を削除
 *    - 改行 (0x0a)をnull (0x00)に変換
 *    - ファイル終了を2つのnullでマーク
 *    - 長さが4バイトの倍数になるようにnullを詰める
 */
static int condense(uchar *buf, int n)
{
    uchar *p, *ep, *lp, *op;
    int c, skipping;

    skipping = 0;       /* true if in a comment */
    ep = buf + n;       /* end of input */
    op = buf;           /* end of output */
    lp = buf;           /* start of current output line */
    for (p = buf; p < ep; p++) {
        switch(c = *p) {
        case '#':
            skipping = 1;
            break;
        case '\0':
        case '\n':
            skipping = 0;
            if (op != lp) {
                *op++ = '\0';
                lp = op;
            }
            break;
        case '\r':
            break;
        default:
            if (!skipping)
                *op++ = c;
            break;
        }
    }
    if (!skipping && op != lp)
        *op++ = '\0';
    *op++ = '\0';
    for (n = op - buf; n & 03; n++)
        *op++ = '\0';
    return n;
}

/*
 * ctl->firmpath: /d/firmware にファームウェアファイルがないか探す
 * 見つからなかった場合はエラーを投げる
 */
static Chan *findfirmware(char *file)
{
    Chan *c;

    if (!waserror()) {
        c = namec(file, Aopen, OREAD, 0);
        poperror();
    } else {
        c = nil;
        snprint(up->genbuf, sizeof up->genbuf, "can't find %s", file);
        p9error(up->genbuf);
    }
    return c;
}

/* ファイル（ファームウェア、config）をアップロードする */
static int upload(Ctlr *ctl, char *file, int isconfig)
{
    Chan *c;
    uchar *buf;
    uchar *cbuf;
    int off, n;

    buf = cbuf = nil;
    c = findfirmware(file);
    if (waserror()) {
        cclose(c);
        sdfree(buf);
        if (Firmwarecmp && cbuf != 0)
            sdfree(cbuf);
        nexterror();
    }
    buf = sdmalloc(Uploadsz);       /* 2KB */
    if (buf == nil)
        p9error(Enomem);
    if (Firmwarecmp) {              /* ロード後にチェックする場合 */
        cbuf = sdmalloc(Uploadsz);  /* 2KB */
        if (cbuf == nil)
            p9error(Enomem);
    }
    off = 0;
    for (;;) {
        // bufにオフセットから2048バイト読み込む
        n = devtab[c->type]->read(c, buf, Uploadsz, off);
        if (n <= 0)
            break;
        if (isconfig) {
            n = condense(buf, n);
            off = ctl->socramsize - n - 4;
        } else if (off == 0) {
            memmove(ctl->resetvec.c, buf, sizeof(ctl->resetvec.c));
        }
        while (n&3) buf[n++] = 0;
        // chipのRAMに書き込む
        if(0)print("write %d bytes to offset %d", n, off);
        sbmem(1, buf, n, ctl->rambase + off);
        if(0)print(" ok\n");
        /* configファイルは2048バイト以内 */
        if (isconfig)
            break;
        off += n;
    }
    if (Firmwarecmp) {
        if (FWDEBUG) print(" compare... ");
        if (!isconfig)
            off = 0;
        for (;;) {
            /* ファームウェアファイルの場合はsdカードからbufに再度読み込む */
            /* configファイルはcondenseしたファイルがbufに残っている */
            if (!isconfig) {
                n = devtab[c->type]->read(c, buf, Uploadsz, off);
                if (n <= 0) {
                    break;
                }
                while (n&3) buf[n++] = 0;
            }
            // ファイルをchipのRAMからcbufに読み込む
            sbmem(0, cbuf, n, ctl->rambase + off);
            // 2つのファイルを比べる
            if (memcmp(buf, cbuf, n) != 0) {
                print("ether4330: firmware load failed offset %d\n", off);
                p9error(Eio);
            }
            if (isconfig)
                break;
            off += n;
        }
    }
    poperror();
    cclose(c);
    sdfree(buf);
    if (Firmwarecmp)
        sdfree(cbuf);
    if (FWDEBUG) print("ok\n");
    return n;
}

#if 0
/* ファームウェアをアップロードする */
static int loadfirm(Ctlr *ctl)
{
    extern char _binary_firmware_bin_start[], _binary_firmware_bin_end[];

    uint8_t *buf = (uint8_t *)_binary_firmware_bin_start;
    uint64_t bsize = (uint64_t)(_binary_firmware_bin_end - _binary_firmware_bin_start);
    uint8_t *cbuf = 0;
    int off, n;

    if (waserror()) {
        if (Firmwarecmp && cbuf != 0)
            sdfree(cbuf);
        nexterror();
    }

    if (Firmwarecmp) {              /* ロード後にチェックする場合 */
        cbuf = sdmalloc(Uploadsz);  /* 2KB */
        if (cbuf == nil)
            p9error(Enomem);
    }

    off = 0;
    for (;;) {
        if (off == 0) {
            memmove(ctl->resetvec.c, buf, sizeof(ctl->resetvec.c));
        }
        n = MIN(Uploadsz, bsize);
        while (n&3) buf[n++] = 0;
        // chipのRAMに書き込む
        sbmem(1, buf+off, n, ctl->rambase + off);
        off += n;
        bsize -= n;
        if (bsize <= 0)
            break;
    }
    if (Firmwarecmp) {
        if (FWDEBUG) print(" compare... ");
        off = 0;
        bsize = (uint64_t)(_binary_firmware_bin_end - _binary_firmware_bin_start);
        for (;;) {
            n = MIN(Uploadsz, bsize);
            // ファイルをchipのRAMからcbufに読み込む
            if(0)print("sbmem n: 0x%x, off: 0x%x\n", n, ctl->rambase + off);
            sbmem(0, cbuf, n, ctl->rambase + off);
            // 2つのファイルを比べる
            if (memcmp(buf+off, cbuf, n) != 0) {
                print("ether4330: firmware load failed offset %d\n", off);
                p9error(Eio);
            }
            off += n;
            bsize -= n;
            if (bsize <= 0)
                break;
        }
    }
    poperror();
    if (Firmwarecmp)
        sdfree(cbuf);
    if (FWDEBUG) print("ok\n");
    return n;
}

/* configをアップロードする */
static int loadconfig(Ctlr *ctl)
{
    uint8_t *buf = brcmfmac43455_sdio_txt;
    int off, n = brcmfmac43455_sdio_txt_len;

    if (waserror()) {
        nexterror();
    }

    n = condense(buf, n);
    off = ctl->socramsize - n - 4;
    while (n&3) buf[n++] = 0;
    // chipのRAMに書き込む
    sbmem(1, buf, n, ctl->rambase + off);
    /* configファイルは2048バイト以内 */
    poperror();
    if (FWDEBUG) print("ok\n");
    return n;
}

/*
 * 規制ファイル (.clm) をファームウェアにアップロードする.
 * パケットフォーマットは次の通り
 *    [2]flag [2]type [4]len [4]crc [len]data
 */
static void reguload2(Ctlr *ctl)
{
    uint8_t *buf = brcmfmac43455_sdio_clm_blob;
    int off, flag, n;
    uint64_t bsize = brcmfmac43455_sdio_clm_blob_len;

    enum {
        Reguhdr = 2+2+4+4,
        Regusz  = 1400,
        Regutyp = 2,
        Flagclm = 1<<12,
        Firstpkt= 1<<1,
        Lastpkt = 1<<2,
    };

    if (waserror()) {
        nexterror();
    }
    put2(buf+2, Regutyp);       /* type */
    put2(buf+8, 0);             /* CRC  */
    off = 0;
    flag = Flagclm | Firstpkt;  /* flag */
    while ((flag & Lastpkt) == 0) {
        n = MIN(Regusz+1, bsize);
        if (n == Regusz+1) {
            --n;
        } else {
            /* 8バイトアライン */
            while (n & 7)
                buf[Reguhdr+n++] = 0;
            flag |= Lastpkt;
        }
        put2(buf+0, flag);      /* flag */
        put4(buf+4, n);         /* len */
        wlsetvar(ctl, "clmload", buf+off, Reguhdr + n);
        off += n;
        flag &= ~Firstpkt;
        bsize -= n;
        if (bsize <= 0)
            break;
    }
    poperror();
}
#endif

static void reguload(Ctlr *ctl, char *file)
{
    Chan *c;
    uchar *buf;
    int off, n, flag;
    enum {
        Reguhdr = 2+2+4+4,
        Regusz  = 1400,
        Regutyp = 2,
        Flagclm = 1<<12,
        Firstpkt= 1<<1,
        Lastpkt = 1<<2,
    };

    buf = nil;
    c = findfirmware(file);
    if (waserror()) {
        cclose(c);
        kmfree(buf);
        nexterror();
    }
    buf = kmalloc(Reguhdr+Regusz+1);
    if (buf == nil)
        p9error(Enomem);
    put2(buf+2, Regutyp);       /* type */
    put2(buf+8, 0);             /* CRC  */
    off = 0;
    flag = Flagclm | Firstpkt;  /* flag */
    while ((flag & Lastpkt) == 0) {
        n = devtab[c->type]->read(c, buf+Reguhdr, Regusz+1, off);
        if (n <= 0)
            break;
        if (n == Regusz+1)
            --n;
        else{
            /* 8バイトアライン */
            while (n & 7)
                buf[Reguhdr+n++] = 0;
            flag |= Lastpkt;
        }
        put2(buf+0, flag);      /* flag */
        put4(buf+4, n);         /* len */
        wlsetvar(ctl, "clmload", buf, Reguhdr + n);
        off += n;
        flag &= ~Firstpkt;
    }
    poperror();
    cclose(c);
    kmfree(buf);
}

/* ファームウェアファイルをアップロード */
static void fwload(Ctlr *ctl)
{
    uchar buf[4];
    uint i, n;

    i = 0;
    while (firmware[i].chipid != ctl->chipid ||
           firmware[i].chiprev != ctl->chiprev) {
        if (++i == nelem(firmware)) {
            print("ether4330: no firmware for chipid %x (%d) chiprev %d\n",
                ctl->chipid, ctl->chipid, ctl->chiprev);
            p9error("no firmware");
        }
    }
    ctl->regufile = firmware[i].regufile;
    /* ALPを要求して、利用可能になるまで待つ */
    cfgw(Clkcsr, ReqALP);
    while ((cfgr(Clkcsr) & ALPavail) == 0)
        microdelay(10);
    memset(buf, 0, 4);
    sbmem(1, buf, 4, ctl->rambase + ctl->socramsize - 4);
    /* ファームウェアファイルをロード */
    if (FWDEBUG) print("firmware %s load... ", firmware[i].fwfile);
    upload(ctl, firmware[i].fwfile, 0);         // brcmfmac43455-sdio.bin
    //loadfirm(ctl);
    /* 構成ファイルをロード*/
    if (FWDEBUG) print("config %s load... ", firmware[i].cfgfile);
    n = upload(ctl, firmware[i].cfgfile, 1);    // brcmfmac43455-sdio.txt
    //n = loadconfig(ctl);
    n /= 4;
    n = (n & 0xFFFF) | (~n << 16);              // n + nの補数をsocramszeの最後の4バイトにセット
    put4(buf, n);
    sbmem(1, buf, 4, ctl->rambase + ctl->socramsize - 4);
    /* チップのSDIOの割り込みをクリアして、cpuをhalt */
    if (ctl->armcore == ARMcr4) {               // 該当しない
        sbwindow(ctl->sdregs);
        cfgwritel(Fn1, ctl->sdregs + Intstatus, ~0);
        if (ctl->resetvec.i != 0) {
            if (SBDEBUG) print("%x\n", ctl->resetvec.i);
            sbmem(1, ctl->resetvec.c, sizeof(ctl->resetvec.c), 0);
        }
        sbreset(ctl->armctl, Cr4Cpuhalt, 0);
    }else
        sbreset(ctl->armctl, 0, 0);
}

/*
 * wlanによるデータパケットと制御パケットの送受信
 */

/* SDIOからの割り込みを処理 */
static void intwait(Ctlr *ctlr, int wait)
{
    ulong ints, mbox;
    int i;

    if (waserror())
        return;
    for (;;) {
        /* SDIOからの割り込みを待機 */
        sdiocardintr(wait);
        sbwindow(ctlr->sdregs);
        i = sdiord(Fn0, Intpend);
        if (i == 0) {
            /* p9tsleep(&up->sleep, return0, 0, 10); */
            continue;
        }
        /* SDIOからの割り込みを取得してクリア */
        ints = cfgreadl(Fn1, ctlr->sdregs + Intstatus);
        cfgwritel(Fn1, ctlr->sdregs + Intstatus, ints);
        if (0) print("INTS: (%x) %lx -> %lx\n", i, ints, cfgreadl(Fn1, ctlr->sdregs + Intstatus));
        /* Mailbox割り込みの場合、Hostmboxdataレジスタの値を読んで、
         * 0x8が立っていればファームウェアの準備がOK */
        if (ints & MailboxInt) {
            mbox = cfgreadl(Fn1, ctlr->sdregs + Hostmboxdata);
            cfgwritel(Fn1, ctlr->sdregs + Sbmbox, 2);    /* ack */
            if (mbox & 0x8)
                print("ether4330: firmware ready\n");
        }
        /* FrameInt割り込みがあれば終了 */
        if (ints & FrameInt)
            break;
    }
    poperror();
}

/* パケットを読み込む */
static Block *wlreadpkt(Ctlr *ctl)
{
    Block *b;
    Sdpcm *p;
    int len, lenck;

    b = allocb(2048);
    p = (Sdpcm*)b->wp;
    qlock(&ctl->pktlock);
    for (;;) {
        /* SDPCMヘッダーを読み込む */
        packetrw(0, b->wp, sizeof(*p));
        len = p->len[0] | p->len[1]<<8;
        /* データ長が0なら処理終了 */
        if (len == 0) {
            freeb(b);
            b = nil;
            break;
        }
        lenck = p->lenck[0] | p->lenck[1]<<8;
        /* データ長のチェック */
        if (lenck != (len ^ 0xFFFF) ||
            len < sizeof(*p) || len > 2048) {
            print("ether4330: wlreadpkt error len %.4x lenck %.4x\n", len, lenck);
            cfgw(Framectl, Rfhalt);
            while (cfgr(Rfrmcnt+1))
                ;
            while (cfgr(Rfrmcnt))
                ;
            continue;
        }
        /* パケットのデータをブロックpのSdpcmヘッダーの後ろに読み込む */
        /* b: SDPCMヘッダー+データ, b->wp = len */
        if (len > sizeof(*p))
            packetrw(0, b->wp + sizeof(*p), len - sizeof(*p));
        b->wp += len;
        break;
    }
    qunlock(&ctl->pktlock);
    return b;
}

/* パケットの送信を開始する */
static void txstart(Ether *edev)
{
    Ctlr *ctl;
    Sdpcm *p;
    Block *b;
    int len, off;

    ctl = edev->ctlr;
    if (!canqlock(&ctl->tlock))
        return;
    if (waserror()) {
        qunlock(&ctl->tlock);
        return;
    }
    for (;;) {
        acquire(&ctl->txwinlock);
        if (ctl->txseq == ctl->txwindow) {
            //print("f");
            release(&ctl->txwinlock);
            break;
        }
        if (ctl->fcmask & 1<<2) {
            //print("x");
            release(&ctl->txwinlock);
            break;
        }
        release(&ctl->txwinlock);
        /* 送信すべきパケットを取り出す */
        b = qget(edev->oq);
        /* 送信すべきパケットがなければ終了 */
        if (b == nil)
            break;
        off = ((uintptr_t)b->rp & 3) + sizeof(Sdpcm);
        b = padblock(b, off + 4);
        len = BLEN(b);
        p = (Sdpcm*)b->rp;
        memset(p, 0, off);          /* TODO: refactor dup code */
        put2(p->len, len);
        put2(p->lenck, ~len);
        p->chanflg = 2;             /* packet */
        p->seq = ctl->txseq;
        p->doffset = off;
        put4(b->rp + off, 0x20);    /* BDC header */
        if (iodebug) dump("send", b->rp, len);
        qlock(&ctl->pktlock);
        if (waserror()) {
            if (iodebug) print("halt frame %x %x\n", cfgr(Wfrmcnt+1), cfgr(Wfrmcnt+1));
            cfgw(Framectl, Wfhalt);
            while (cfgr(Wfrmcnt+1))
                ;
            while (cfgr(Wfrmcnt))
                ;
            qunlock(&ctl->pktlock);
            nexterror();
        }
        /* 実際にパケットを送信 */
        packetrw(1, b->rp, len);
        ctl->txseq++;
        poperror();
        qunlock(&ctl->pktlock);
        freeb(b);
    }
    poperror();
    qunlock(&ctl->tlock);
}

/* カーネルスレッドに実行させるパケット受信処理関数 */
static void rproc(void *a)
{
    Ether *edev;
    Ctlr *ctl;
    Block *b;
    Sdpcm *p;
    Cmd *q;
    int flowstart;
    int bdc;

    edev = a;
    ctl = edev->ctlr;
    flowstart = 0;
    for (;;) {
        if (flowstart) {
            //print("F");
            flowstart = 0;
            txstart(edev);      // wlanチップへの送信開始
        }
        b = wlreadpkt(ctl);     // wlanチップからのパケットread
        if (b == nil) {
            intwait(ctl, 1);
            continue;
        }
        p = (Sdpcm*)b->rp;
        /* creditのチェック: 転送開始するかをチェック */
        if (p->window != ctl->txwindow || p->fcmask != ctl->fcmask) {
            acquire(&ctl->txwinlock);
            if (p->window != ctl->txwindow) {
                if (ctl->txseq == ctl->txwindow)
                    flowstart = 1;
                ctl->txwindow = p->window;
            }
            if (p->fcmask != ctl->fcmask) {
                if ((p->fcmask & 1<<2) == 0)
                    flowstart = 1;
                ctl->fcmask = p->fcmask;
            }
            release(&ctl->txwinlock);
        }
        switch(p->chanflg & 0xF) {
        case 0:    /* rsp : 応答 */
            if (iodebug) dump("rsp", b->rp, BLEN(b));
            if (BLEN(b) < sizeof(Sdpcm) + sizeof(Cmd))
                break;
            q = (Cmd*)(b->rp + sizeof(*p));     // CDC
            if ((q->id[0] | q->id[1]<<8) != ctl->reqid) // リクエストの応答でない
                break;
            ctl->rsp = b;           // リクエストの応答（ヘッダーは削除していない）
            p9wakeup(&ctl->cmdr);   // リクエストの応答を待ってsleepしているプロセスを起床
            continue;
        case 1:    /* event : イベント */
            if (iodebug) dump("event", b->rp, BLEN(b));
            if (BLEN(b) > p->doffset + 4) {     // sdpcmヘッダー + BDCヘッダー
                bdc = 4 + (b->rp[p->doffset + 3] << 2); // BDCヘッダーの末尾からパケットデータまでのオフセットは4 uint8_tワード単位。
                if (BLEN(b) > p->doffset + bdc) {
                    b->rp += p->doffset + bdc;    /* BDC ヘッダーをスキップ */
                    trace("bcmevent: len=0x%x", BLEN(b));
                    bcmevent(ctl, b->rp, BLEN(b));
                    break;
                }
            }
            if (iodebug && BLEN(b) != p->doffset)
                print("short event %ld %d\n", BLEN(b), p->doffset);
            break;
        case 2:    /* packet : イーサネットパケット */
            if (iodebug) dump("packet", b->rp, BLEN(b));
            if (BLEN(b) > p->doffset + 4) {
                bdc = 4 + (b->rp[p->doffset + 3] << 2);
                if (BLEN(b) >= p->doffset + bdc + ETHERHDRSIZE) {
                    b->rp += p->doffset + bdc;  /* BDCヘッダーをスキップ */
                    etheriq(edev, b, 1);        /* bの先頭はEthernetヘッダー : bcm434.cpp で定義 */
                    continue;
                }
            }
            break;
        default:
            dump("ether4330: bad packet", b->rp, BLEN(b));
            break;
        }
        freeb(b);
    }
}

/* linkdown する */
static void linkdown(Ctlr *ctl)
{
    Ether *edev;

    edev = ctl->edev;
    if (edev == nil || ctl->status != Connected)
        return;
    ctl->status = Disconnected;
    memset(ctl->bssid, 0, Eaddrlen);
}

/*
 * ホストとファームウェア間のコマンドインタフェース
 */

static char *eventnames[] = {
    [0] = "set ssid",
    [1] = "join",
    [2] = "start",
    [3] = "auth",
    [4] = "auth ind",
    [5] = "deauth",
    [6] = "deauth ind",
    [7] = "assoc",
    [8] = "assoc ind",
    [9] = "reassoc",
    [10] = "reassoc ind",
    [11] = "disassoc",
    [12] = "disassoc ind",
    [13] = "quiet start",
    [14] = "quiet end",
    [15] = "beacon rx",
    [16] = "link",
    [17] = "mic error",
    [18] = "ndis link",
    [19] = "roam",
    [20] = "txfail",
    [21] = "pmkid cache",
    [22] = "retrograde tsf",
    [23] = "prune",
    [24] = "autoauth",
    [25] = "eapol msg",
    [26] = "scan complete",
    [27] = "addts ind",
    [28] = "delts ind",
    [29] = "bcnsent ind",
    [30] = "bcnrx msg",
    [31] = "bcnlost msg",
    [32] = "roam prep",
    [33] = "pfn net found",
    [34] = "pfn net lost",
    [35] = "reset complete",
    [36] = "join start",
    [37] = "roam start",
    [38] = "assoc start",
    [39] = "ibss assoc",
    [40] = "radio",
    [41] = "psm watchdog",
    [44] = "probreq msg",
    [45] = "scan confirm ind",
    [46] = "psk sup",
    [47] = "country code changed",
    [48] = "exceeded medium time",
    [49] = "icv error",
    [50] = "unicast decode error",
    [51] = "multicast decode error",
    [52] = "trace",
    [53] = "bta hci event",
    [54] = "if",
    [55] = "p2p disc listen complete",
    [56] = "rssi",
    [57] = "pfn scan complete",
    [58] = "extlog msg",
    [59] = "action frame",
    [60] = "action frame complete",
    [61] = "pre assoc ind",
    [62] = "pre reassoc ind",
    [63] = "channel adopted",
    [64] = "ap started",
    [65] = "dfs ap stop",
    [66] = "dfs ap resume",
    [67] = "wai sta event",
    [68] = "wai msg",
    [69] = "escan result",
    [70] = "action frame off chan complete",
    [71] = "probresp msg",
    [72] = "p2p probreq msg",
    [73] = "dcs request",
    [74] = "fifo credit map",
    [75] = "action frame rx",
    [76] = "wake event",
    [77] = "rm complete",
    [78] = "htsfsync",
    [79] = "overlay req",
    [80] = "csa complete ind",
    [81] = "excess pm wake event",
    [82] = "pfn scan none",
    [83] = "pfn scan allgone",
    [84] = "gtk plumbed",
    [85] = "assoc ind ndis",
    [86] = "reassoc ind ndis",
    [87] = "assoc req ie",
    [88] = "assoc resp ie",
    [89] = "assoc recreated",
    [90] = "action frame rx ndis",
    [91] = "auth req",
    [92] = "tdls peer event",
    [127] = "bcmc credit support"
};

/* イベント番号を文字列に変更する */
static char *evstring(uint event)
{
    static char buf[12];

    if (event >= nelem(eventnames) || eventnames[event] == 0) {
        /* not reentrant but only called from one kproc */
        snprint(buf, sizeof buf, "%d", event);
        return buf;
    }
    return eventnames[event];
}

/* 受信したイベント要求を処理 */
static void bcmevent(Ctlr *ctl, uchar *p, int len)
{
    int flags;
    long event, status, reason;
    ether_event_params_t params;

    memset (&params, 0, sizeof params);

    if (len < ETHERHDRSIZE + 10 + 46)
        return;
    //hexdump(p, 64, "event data");
    p += ETHERHDRSIZE + 10;             /* skip bcm_ether header */
    len -= ETHERHDRSIZE + 10;
    flags = nhgets(p + 2);              /* whd_events.h の struct whd_event_msg を参照 */
    event = nhgets(p + 6);
    status = nhgetl(p + 8);
    reason = nhgetl(p + 12);
    if (EVENTDEBUG)
        print("ether4330: [%s] status %ld flags 0x%x reason %ld\n",
            evstring(event), status, flags, reason);
    switch(event) {
    case 19:    /* E_ROAM */
        if (status == 0)
            break;
    /* fall through */
    case 0:        /* E_SET_SSID */
        memcpy(ctl->bssid, p + 24, Eaddrlen);
        ctl->joinstatus = 1 + status;
        p9wakeup(&ctl->joinr);
        break;
    case 5:        /* E_DEAUTH */
    case 6:        /* E_DEAUTH_IND */
        linkdown(ctl);
        callevhndlr(ctl, ether_event_deauth, 0);
        break;
    case 16:    /* E_LINK */
        if (flags & 1) {    /* link up */
            callevhndlr(ctl, ether_event_link, 0);
            break;
        }
    /* fall through */
    case 12:    /* E_DISASSOC_IND */
        linkdown(ctl);
        callevhndlr(ctl, ether_event_disassoc, 0);
        break;
    case 17:    /* E_MIC_ERROR */
        params.mic_error.group = !!(flags & 4);
        memcpy(params.mic_error.addr, p + 24, Eaddrlen);
        callevhndlr(ctl, ether_event_mic_error, &params);
        break;
    case 3:     /* E_AUTH */
    case 26:    /* E_SCAN_COMPLETE */
        break;
    case 69:    /* E_ESCAN_RESULT */
        //hexdump(p, len, "escan_result");
        wlscanresult(ctl->edev, p + 48, len - 48);
        break;
    default:
        if (status) {
            if (!EVENTDEBUG)
                print("ether4330: [%s] error status %ld flags %#x reason %ld\n",
                    evstring(event), status, flags, reason);
            //dump("event", p, len);
        }
    }
}

/* 接続状態を返す */
static int joindone(void *a)
{
    return ((Ctlr*)a)->joinstatus;
}

/* 接続されるまで待機 */
static int waitjoin(Ctlr *ctl)
{
    int n;

    p9sleep(&ctl->joinr, joindone, ctl);
    n = ctl->joinstatus;
    ctl->joinstatus = 0;
    return n - 1;
}

/* コマンドが終了した際に実行するハンドラ関数 */
static int cmddone(void *a)
{
    return ((Ctlr*)a)->rsp != nil;
}

/* ファームウェアにコマンドopを実行させる */
static void wlcmd(Ctlr *ctl, int write, int op, void *data, int dlen, void *res, int rlen)
{
    Block *b;
    Sdpcm *p;
    Cmd *q;
    int len, tlen;

    if (write)
        tlen = dlen + rlen;
    else
        tlen = MAX(dlen, rlen);
    len = sizeof(Sdpcm) + sizeof(Cmd) + tlen;
    b = allocb(len);
    qlock(&ctl->cmdlock);
    if (waserror()) {
        freeb(b);
        qunlock(&ctl->cmdlock);
        nexterror();
    }
    memset(b->wp, 0, len);      /* wpからwp+lenまで0クリア */
    qlock(&ctl->pktlock);
    p = (Sdpcm*)b->wp;          /* p (SDPCMヘッダ構造体) をセット */
    put2(p->len, len);
    put2(p->lenck, ~len);       /* lenckはlenの補数 */
    p->seq = ctl->txseq;
    p->doffset = sizeof(Sdpcm); /* データオフセット : SDPCMヘッダ長 */
    b->wp += sizeof(*p);        /* SDPCMヘッダの次のバイトにwpを移動する */

    q = (Cmd*)b->wp;            /* q (CDCヘッダ構造体) をセット */
    put4(q->cmd, op);
    put4(q->len, tlen);         /* 下位16ビット: dlen, 上位16ビット: rlen */
    put2(q->flags, write? 2 : 0);
    put2(q->id, ++ctl->reqid);
    put4(q->status, 0);
    b->wp += sizeof(*q);        /* CDCヘッダの次のバイトにwpを移動する */

    /* dataをwpにコピー */
    if (dlen > 0)
        memmove(b->wp, data, dlen);
    if (write)
        memmove(b->wp + dlen, res, rlen);
    b->wp += tlen;              /* tlenの次のバイトにwpを移動する */

    if (iodebug) dump("cmd", b->rp, len);
    /* ブロックの先頭からlenだけ書き込み */
    packetrw(1, b->rp, len);
    ctl->txseq++;
    qunlock(&ctl->pktlock);
    freeb(b);
    b = nil;
    USED(b);

    /* 書き込みが終わるのを待機してsleep */
    p9sleep(&ctl->cmdr, cmddone, ctl);
    /* 応答をブロックにセット */
    b = ctl->rsp;
    ctl->rsp = nil;
    assert(b != nil);
    p = (Sdpcm*)b->rp;
    q = (Cmd*)(b->rp + p->doffset);
    if (q->status[0] | q->status[1] | q->status[2] | q->status[3]) {
        print("ether4330: cmd %d error status %ld\n", op, get4(q->status));
        dump("ether4330: cmd error", b->rp, BLEN(b));
        p9error("wlcmd error");
    }
    /* 読み込みの場合はCDCヘッダ以降をresにコピーする */
    if (!write)
        memmove(res, q + 1, rlen);
    freeb(b);
    qunlock(&ctl->cmdlock);
    poperror();
}

/* valを引数にコマンドopの処理する */
static void wlcmdint(Ctlr *ctl, int op, int val)
{
    uchar buf[4];

    put4(buf, val);
    wlcmd(ctl, 1, op, buf, 4, nil, 0);
}

/* 変数nameの値を取得する */
static void wlgetvar(Ctlr *ctl, char *name, void *val, int len)
{
    wlcmd(ctl, 0, GetVar, name, strlen(name) + 1, val, len);
}

/* 変数nameに長さlenの値をセットする */
static void wlsetvar(Ctlr *ctl, char *name, void *val, int len)
{
    if (VARDEBUG) {
        char buf[32];
        snprint(buf, sizeof buf, "wlsetvar %s:", name);
        dump(buf, val, len);
    }
    wlcmd(ctl, 1, SetVar, name, strlen(name) + 1, val, len);
}

/* 変数nameに4バイトの値をセットする */
static void wlsetint(Ctlr *ctl, char *name, int val)
{
    uchar buf[4];

    put4(buf, val);
    wlsetvar(ctl, name, buf, 4);
}

/* WEPキーワードctl->keys[i]を変数wsec_keyにセットする */
static void wlwepkey(Ctlr *ctl, int i)
{
    uchar params[164];
    uchar *p;

    memset(params, 0, sizeof params);
    p = params;
    p = put4(p, i);         /* index */
    p = put4(p, ctl->keys[i].len);
    memmove(p, ctl->keys[i].dat, ctl->keys[i].len);
    p += 32 + 18*4;         /* keydata, pad */
    if (ctl->keys[i].len == WMinKeyLen)
        p = put4(p, 1);     /* algo = WEP1 */
    else
        p = put4(p, 3);     /* algo = WEP128 */
    put4(p, 2);             /* flags = Primarykey */

    wlsetvar(ctl, "wsec_key", params, sizeof params);
}

/* WPAキーワードを変数wsec_keyにセットする */
static void wlwpakey(Ctlr *ctl, int id, uvlong iv, uchar *ea)
{
    uchar params[164];
    uchar *p;
    int pairwise;

    if (id == CMrxkey)
        return;
    pairwise = (id == CMrxkey || id == CMtxkey);
    memset(params, 0, sizeof params);
    p = params;
    if (pairwise)
        p = put4(p, 0);
    else
        p = put4(p, id - CMrxkey0); /* group key id */
    p = put4(p, ctl->keys[0].len);
    memmove((char*)p,  ctl->keys[0].dat, ctl->keys[0].len);
    p += 32 + 18*4;                 /* keydata, pad */
    if (ctl->cryptotype == Wpa)
        p = put4(p, 2);             /* algo = TKIP */
    else
        p = put4(p, 4);             /* algo = AES_CCM */
    if (pairwise)
        p = put4(p, 0);
    else
        p = put4(p, 2);             /* flags = Primarykey */
    p += 3*4;
    p = put4(p, 0);                 /* iv initialised */
    p += 4;
    p = put4(p, iv>>16);            /* iv high */
    p = put2(p, iv&0xFFFF);         /* iv low */
    p += 2 + 2*4;                   /* align, pad */
    if (pairwise)
        memmove(p, ea, Eaddrlen);

    wlsetvar(ctl, "wsec_key", params, sizeof params);
}

/* 名前がssidのAPに接続する */
static void wljoin(Ctlr *ctl, char *ssid, int chan, uchar *bssid)
{
    uchar params[72];
    uchar *p;
    int n;

    if (chan != 0)
        chan |= 0x2b00;     /* 20Mhz channel width */
    p = params;
    n = strlen(ssid);
    n = MIN(n, 32);
    p = put4(p, n);
    memmove(p, ssid, n);
    memset(p + n, 0, 32 - n);
    p += 32;
    p = put4(p, 0xff);      /* scan type */
    if (chan != 0) {
        p = put4(p, 2);     /* num probes */
        p = put4(p, 120);   /* active time */
        p = put4(p, 390);   /* passive time */
    } else {
        p = put4(p, -1);    /* num probes */
        p = put4(p, -1);    /* active time */
        p = put4(p, -1);    /* passive time */
    }
    p = put4(p, -1);        /* home time */
    if (bssid != 0)
        memcpy(p, bssid, Eaddrlen);     /* bssid */
    else
        memset(p, 0xFF, Eaddrlen);
    p += Eaddrlen;
    p = put2(p, 0);         /* pad */
    if (chan != 0) {
        p = put4(p, 1);     /* num chans */
        p = put2(p, chan);  /* chan spec */
        p = put2(p, 0);     /* pad */
        assert(p == params + sizeof(params));
    } else {
        p = put4(p, 0);     /* num chans */
        assert(p == params + sizeof(params) - 4);
    }

    wlsetvar(ctl, "join", params, chan? sizeof params : sizeof params - 4);
    ctl->status = Connecting;
    switch(waitjoin(ctl)) {
        case 0:
            ctl->status = Connected;
            break;
        case 3:
            ctl->status = Disconnected;
            p9error("wifi join: network not found");
        case 1:
            ctl->status = Disconnected;
            p9error("wifi join: failed");
        default:
            ctl->status = Disconnected;
            p9error("wifi join: error");
    }
}

/* APを作成する */
static void wlcreateAP(Ctlr *ctl, char *ssid, int channel, int hidden)    /* by @sebastienNEC */
{
    wlcmdint(ctl, 3, 1);        /* DOWN */
    wlcmdint(ctl, 20, 1);       /* SET_INFRA */
    wlcmdint(ctl, 118, 1);      /* SET_AP */
    wlcmdint(ctl, 30, channel);
    wlcmdint(ctl, 2, 1);        /* UP */

    uchar join_params[4+WNameLen+14];
    uchar *p = join_params;
    int n = strlen(ssid);       /* copy ssid */
    n = MIN(n, WNameLen);
    p = put4(p, n);
    memmove(p, ssid, n);
    memset(p + n, 0, WNameLen - n);
    p += WNameLen;
    memset(p, 0, 14);           /* clear assoc params */
    wlcmd(ctl, 1, 26, &join_params, sizeof(join_params), nil, 0);    /* SET_SSID */

    wlsetint(ctl, "closednet", hidden);

    /* TODO? beacon settings */

    ctl->status = Connected;    /* TODO: check return code as in waitjoin() */
}

/* APスキャンを開始する */
static void wlscanstart(Ctlr *ctl)
{
    /* version[4] action[2] sync_id[2] ssidlen[4] ssid[32] bssid[6] bss_type[1]
        scan_type[1] nprobes[4] active_time[4] passive_time[4] home_time[4]
        nchans[2] nssids[2] chans[nchans][2] ssids[nssids][32] */
    /* hack - this is only correct on a little-endian cpu */
    static uchar params[4+2+2+4+32+6+1+1+4*4+2+2+14*2+32+4] = {
        1,0,0,0,
        1,0,
        0x34,0x12,
        0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0xff,0xff,0xff,0xff,0xff,0xff,
        2,
        0,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,
        14,0,
        1,0,
        0x01,0x2b,0x02,0x2b,0x03,0x2b,0x04,0x2b,0x05,0x2e,0x06,0x2e,0x07,0x2e,
        0x08,0x2b,0x09,0x2b,0x0a,0x2b,0x0b,0x2b,0x0c,0x2b,0x0d,0x2b,0x0e,0x2b,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

    wlcmdint(ctl, 49, 0);    /* PASSIVE_SCAN */
    wlsetvar(ctl, "escan", params, sizeof params);
}

/* APスキャン結果をセットする */
static void wlscanresult(Ether *edev, uchar *p, int len)
{
    etherscanresult(edev, p, len);
}

/* 国コードをセットする */
static void wlsetcountry(Ctlr *ctlr, const char *ccode)
{
    struct {
        char country_ie[4];
        uint revision;
        char country_code[4];
    } params;

    if (   !('A' <= ccode[0] && ccode[0] <= 'Z')
        || !('A' <= ccode[1] && ccode[1] <= 'Z')
        || ccode[2] != '\0') {
        p9error("Invalid country code");
    }

    strcpy(params.country_ie, ccode);
    strcpy(params.country_code, ccode);
    params.revision = (uint) -1;
    debug("ccode: %s, country: %s", ccode, params.country_code);
    wlsetvar(ctlr, "country", &params, sizeof params);
}

/* カーネルプロセスに実行させるAPスキャン処理関数 */
static void lproc(void *a)
{
    Ether *edev;
    Ctlr *ctlr;
    int secs;

    edev = a;
    ctlr = edev->ctlr;
    secs = 0;
    for (;;) {
        p9tsleep(&up->sleep, return0, 0, 1000);
        if (ctlr->scansecs) {
            if (secs == 0) {
                if (waserror())
                    ctlr->scansecs = 0;
                else{
                    wlscanstart(ctlr);
                    poperror();
                }
                secs = ctlr->scansecs;
            }
            --secs;
        } else {
            secs = 0;
        }
    }
}

/* wlanを初期化する */
static void wlinit(Ether *edev, Ctlr *ctlr)
{
    uchar ea[Eaddrlen];
    uchar eventmask[16];
    char version[128];
    char *p;
    static uchar keepalive[12] = {1, 0, 11, 0, 0xd8, 0xd6, 0, 0, 0, 0, 0, 0};

    wlgetvar(ctlr, "cur_etheraddr", ea, Eaddrlen);
    memmove(edev->ea, ea, Eaddrlen);
    memmove(edev->addr, ea, Eaddrlen);
    print("ether4330: addr %02x:%02x:%02x:%02x:%02x:%02x\n",
          ea[0], ea[1], ea[2], ea[3], ea[4], ea[5]);
    wlsetint(ctlr, "assoc_listen", 10);
    if (ctlr->chipid == 43430 || ctlr->chipid == 0x4345)
        wlcmdint(ctlr, 0x56, 0);    /* powersave off : 該当 */
    else
        wlcmdint(ctlr, 0x56, 2);    /* powersave FAST */
    wlsetint(ctlr, "bus:txglom", 0);
    wlsetint(ctlr, "bcn_timeout", 10);
    wlsetint(ctlr, "assoc_retry_max", 3);
    if (ctlr->chipid == 0x4330) {
        wlsetint(ctlr, "btc_wire", 4);
        wlsetint(ctlr, "btc_mode", 1);
        wlsetvar(ctlr, "mkeep_alive", keepalive, 11);
    }
    memset(eventmask, 0xFF, sizeof eventmask);
#define ENABLE(n)    eventmask[n/8] |= 1<<(n%8)
#define DISABLE(n)   eventmask[n/8] &= ~(1<<(n%8))
    DISABLE(40);    /* E_RADIO */
    DISABLE(44);    /* E_PROBREQ_MSG */
    DISABLE(54);    /* E_IF */
    DISABLE(71);    /* E_PROBRESP_MSG */
    DISABLE(20);    /* E_TXFAIL */
    DISABLE(124);   /* ? */
    wlsetvar(ctlr, "event_msgs", eventmask, sizeof eventmask);
    wlcmdint(ctlr, 0xb9, 0x28);     /* SET_SCAN_CHANNEL_TIME */
    wlcmdint(ctlr, 0xbb, 0x28);     /* SET_SCAN_UNASSOC_TIME */
    wlcmdint(ctlr, 0x102, 0x82);    /* SET_SCAN_PASSIVE_TIME */
    wlcmdint(ctlr, 2, 0);           /* UP */
    memset(version, 0, sizeof version);
    wlgetvar(ctlr, "ver", version, sizeof version - 1);
    if ((p = strchr(version, '\n')) != nil)
        *p = '\0';
    if (0) print("ether4330: %s\n", version);
    wlsetint(ctlr, "roam_off", 1);
    wlcmdint(ctlr, 0x14, 1);        /* SET_INFRA 1 */
    wlcmdint(ctlr, 10, 0);          /* SET_PROMISC */
    //wlcmdint(ctlr, 0x8e, 0);      /* SET_BAND 0 */
    //wlsetint(ctlr, "wsec", 1);
    wlcmdint(ctlr, 2, 1);           /* UP */
    ctlr->keys[0].len = WMinKeyLen;
    //wlwepkey(ctlr, 0);
}

/*
 * Plan 9 ドライバインタフェース
 */

/* ifstatをaにコピー */
static long etherbcmifstat(Ether* edev, void* a, long n, ulong offset)
{
    Ctlr *ctlr;
    char *p;
    int l;
    static char *cryptoname[4] = {
        [0]     "off",
        [Wep]   "wep",
        [Wpa]   "wpa",
        [Wpa2]  "wpa2",
    };
    /* these strings are known by aux/wpa */
    static char* connectstate[] = {
        [Disconnected]  = "unassociated",
        [Connecting]    = "connecting",
        [Connected]     = "associated",
    };

    ctlr = edev->ctlr;
    if (ctlr == nil)
        return 0;
    p = kmalloc(READSTR);
    l = 0;

    l += snprint(p+l, READSTR-l, "channel: %d\n", ctlr->chanid);
    l += snprint(p+l, READSTR-l, "bssid: %02x:%02x:%02x:%02x:%02x:%02x\n",
             ctlr->bssid[0], ctlr->bssid[1], ctlr->bssid[2],
             ctlr->bssid[3], ctlr->bssid[4], ctlr->bssid[5]);
    l += snprint(p+l, READSTR-l, "essid: %s\n", ctlr->essid);
    l += snprint(p+l, READSTR-l, "crypt: %s\n", cryptoname[ctlr->cryptotype]);
    l += snprint(p+l, READSTR-l, "oq: %d\n", qlen(edev->oq));
    l += snprint(p+l, READSTR-l, "txwin: %d\n", ctlr->txwindow);
    l += snprint(p+l, READSTR-l, "txseq: %d\n", ctlr->txseq);
    l += snprint(p+l, READSTR-l, "status: %s\n", connectstate[ctlr->status]);
    USED(l);
    /* pからaへコピー */
    n = readstr(offset, a, n, p);
    kmfree(p);
    return n;
}

/* Ethernet送信を開始 */
static void etherbcmtransmit(Ether *edev)
{
    Ctlr *ctlr;

    ctlr = edev->ctlr;
    if (ctlr == nil)
        return;
    txstart(edev);
}

/* aをhexとしてパースしてbufにセット */
static int parsehex(char *buf, int buflen, char *a)
{
    int i, k, n;

    k = 0;
    for (i = 0; k < buflen && *a; i++) {
        if (*a >= '0' && *a <= '9')
            n = *a++ - '0';
        else if (*a >= 'a' && *a <= 'f')
            n = *a++ - 'a' + 10;
        else if (*a >= 'A' && *a <= 'F')
            n = *a++ - 'A' + 10;
        else
            break;

        if (i & 1) {
            buf[k] |= n;
            k++;
        } else {
            buf[k] = n<<4;
        }
    }
    if (i & 1)
        return -1;
    return k;
}

/* WEPキーワードをパースしてkey->datにセットする */
static int wepparsekey(WKey* key, char* a)
{
    int i, k, len, n;
    char buf[WMaxKeyLen];

    len = strlen(a);
    if (len == WMinKeyLen || len == WMaxKeyLen) {
        memset(key->dat, 0, sizeof(key->dat));
        memmove(key->dat, a, len);
        key->len = len;

        return 0;
    } else if (len == WMinKeyLen*2 || len == WMaxKeyLen*2) {
        k = 0;
        for (i = 0; i < len; i++) {
            if (*a >= '0' && *a <= '9')
                n = *a++ - '0';
            else if (*a >= 'a' && *a <= 'f')
                n = *a++ - 'a' + 10;
            else if (*a >= 'A' && *a <= 'F')
                n = *a++ - 'A' + 10;
            else
                return -1;

            if (i & 1) {
                buf[k] |= n;
                k++;
            } else {
                buf[k] = n<<4;
            }
        }

        memset(key->dat, 0, sizeof(key->dat));
        memmove(key->dat, buf, k);
        key->len = k;

        return 0;
    }

    return -1;
}

/* WPAキーワードをパースしてkey->datにセットする */
static int wpaparsekey(WKey *key, uvlong *ivp, char *a)
{
    int len;
    char *e;

    /* 大文字小文字を無視して比較 */
    if (cistrncmp(a, "tkip:", 5) == 0 || cistrncmp(a, "ccmp:", 5) == 0)
        a += 5;
    else
        return 1;
    len = parsehex(key->dat, sizeof(key->dat), a);
    if (len <= 0)
        return 1;
    key->len = len;
    a += 2*len;
    if (*a++ != '@')
        return 1;
    *ivp = strtoull(a, &e, 16);
    if (e == a)
        return -1;
    return 0;
}

/* aをwpaieとしてパースしてauth関連の変数にセットする */
static void setauth(Ctlr *ctlr, Cmdbuf *cb, char *a)
{
    uchar wpaie[32];
    int i;

    i = parsehex((char*)wpaie, sizeof wpaie, a);
    if (i < 2 || i != wpaie[1] + 2)
        cmderror(cb, "bad wpa ie syntax");
    if (wpaie[0] == 0xdd)
        ctlr->cryptotype = Wpa;
    else if (wpaie[0] == 0x30)
        ctlr->cryptotype = Wpa2;
    else
        cmderror(cb, "bad wpa ie");
    wlsetvar(ctlr, "wpaie", wpaie, i);
    if (ctlr->cryptotype == Wpa) {
        wlsetint(ctlr, "wpa_auth", 4|2);        /* auth_psk | auth_unspecified */
        wlsetint(ctlr, "auth", 0);
        wlsetint(ctlr, "wsec", 2);              /* tkip */
        wlsetint(ctlr, "wpa_auth", 4);          /* auth_psk */
    } else {
        wlsetint(ctlr, "wpa_auth", 0x80|0x40);  /* auth_psk | auth_unspecified */
        wlsetint(ctlr, "auth", 0);
        wlsetint(ctlr, "wsec", 4);              /* aes */
        wlsetint(ctlr, "wpa_auth", 0x80);       /* auth_psk */
    }
}

/* WEPの暗号化方式をチェックしてctlr->cryptotype, auth変数にセットする */
static int setcrypt(Ctlr *ctlr, Cmdbuf*cb, char *a)
{
    if (cistrcmp(a, "wep") == 0 || cistrcmp(a, "on") == 0)
        ctlr->cryptotype = Wep;
    else if (cistrcmp(a, "off") == 0 || cistrcmp(a, "none") == 0)
        ctlr->cryptotype = 0;
    else
        return 0;
    wlsetint(ctlr, "auth", ctlr->cryptotype);
    return 1;
}

/* ファームウェアコマンドを実行する */
static long etherbcmctl(Ether* edev, const void* buf, long n)
{
    Ctlr *ctlr;
    Cmdbuf *cb;
    Cmdtab *ct;
    uchar ea[Eaddrlen];
    uvlong iv = 0;
    int i;

    if ((ctlr = edev->ctlr) == nil)
        p9error(Enonexist);
    USED(ctlr);

    cb = parsecmd(buf, n);
    if (waserror()) {
        kmfree(cb);
        nexterror();
    }
    // cbで示されたコマンドを取り出す
    ct = lookupcmd(cb, cmds, nelem(cmds));
    switch(ct->index) {
    case CMauth:
        /* cbの第一引数をwpaキーワードしてセットする */
        setauth(ctlr, cb, cb->f[1]);
        /* アクセスポイントが指定されていたら接続する */
        if (ctlr->essid[0])
            wljoin(ctlr, ctlr->essid, ctlr->chanid, 0);
        break;
    case CMchannel:
        /* cbの第一引数をctlr->chanidにセットする */
        if ((i = atoi(cb->f[1])) < 0 || i > 16)
            cmderror(cb, "bad channel number");
        //wlcmdint(ctlr, 30, i);    /* SET_CHANNEL */
        ctlr->chanid = i;
        break;
    case CMcrypt:
        /* cbの第一引数を暗号化したキーワードしてセットする */
        if (setcrypt(ctlr, cb, cb->f[1])) {
            if (ctlr->essid[0])
                wljoin(ctlr, ctlr->essid, ctlr->chanid, 0);
        } else{
            cmderror(cb, "bad crypt type");
        }
        break;
    case CMessid:
        /* cbの第一引数が"default"だったら0を、それ以外は引数をessidとしてセットする*/
        if (cistrcmp(cb->f[1], "default") == 0)
            memset(ctlr->essid, 0, sizeof(ctlr->essid));
        else{
            strncpy(ctlr->essid, cb->f[1], sizeof(ctlr->essid) - 1);
            ctlr->essid[sizeof(ctlr->essid) - 1] = '\0';
        }
        /* エラーでなければそのapに接続する */
        if (!waserror()) {
            wljoin(ctlr, ctlr->essid, ctlr->chanid, 0);
            poperror();
        }
        break;
    case CMjoin:    /* join essid bssid channel wep|on|off|wpakey */
        /* 第1引数はessid, 第2引数はbssid, 第3引数はチャネル番号 */
        /* 第4変数は暗号タイプ, essidが"default"でなければそのAPに接続 */
        if (strcmp(cb->f[1], "") != 0) {    /* empty string for no change */
            if (cistrcmp(cb->f[1], "default") != 0) {
                strncpy(ctlr->essid, cb->f[1], sizeof(ctlr->essid)-1);
                ctlr->essid[sizeof(ctlr->essid)-1] = 0;
            } else {
                memset(ctlr->essid, 0, sizeof(ctlr->essid));
            }
        } else if (ctlr->essid[0] == 0) {
            cmderror(cb, "essid not set");
        }
        if (parseether(ea, cb->f[2]) < 0)
            cmderror(cb, "bad bssid");
        if ((i = atoi(cb->f[3])) >= 0 && i <= 16)
            ctlr->chanid = i;
        else
            cmderror(cb, "bad channel number");
        /* WEP/暗号なし */
        if (!setcrypt(ctlr, cb, cb->f[4]))
            /* WPA */
            setauth(ctlr, cb, cb->f[4]);
        if (ctlr->essid[0])
            wljoin(ctlr, ctlr->essid, ctlr->chanid, ea);
        break;
    case CMkey1:
    case CMkey2:
    case CMkey3:
    case CMkey4:
        /* 第1引数をWEPキーワードとしてパース、エラーがなければwset, wsec_keyにセット */
        i = ct->index - CMkey1;
        if (wepparsekey(&ctlr->keys[i], cb->f[1]))
            cmderror(cb, "bad WEP key syntax");
        wlsetint(ctlr, "wsec", 1);    /* wep enabled */
        wlwepkey(ctlr, i);
        break;
    case CMrxkey:
    case CMrxkey0:
    case CMrxkey1:
    case CMrxkey2:
    case CMrxkey3:
    case CMtxkey:
        /* 第1引数をethernetアドレス、第2引数をWPAキーワードとして */
        /* パースしてwec_keyにセット */
        if (parseether(ea, cb->f[1]) < 0)
            cmderror(cb, "bad ether addr");
        if (wpaparsekey(&ctlr->keys[0], &iv, cb->f[2]))
            cmderror(cb, "bad wpa key");
        wlwpakey(ctlr, ct->index, iv, ea);
        break;
    case CMdisassoc:        /* disassoc reason */
        /* APとの接続を切る */
        if (ctlr->status != Disconnected)
            wlcmdint(ctlr, 52, atoi(cb->f[1]));    /* DISASSOC */
        break;
    case CMescan:           /* escan seconds */
        /* 第1引数をタイムアウト時間としてescanを行う */
        etherbcmscan(edev, atoi(cb->f[1]));
        break;
    case CMcountry:         /* country alpha2 */
        /* 第1引数を国コードしてセットする */
        wlsetcountry(ctlr, cb->f[1]);
        break;
    case CMdebug:
        /* 第1引数をデバッグ指示とする */
        iodebug = atoi(cb->f[1]);
        break;
    case CMcreate:          /* create essid channel 1/0 */ /* by @sebastienNEC */
        /* 第1引数をessid, 第2引数をチャネル番号、第3引数を公開/非公開フラグとして */
        /* APを作成する */
        if (strcmp(cb->f[1], "") != 0) {    /* empty string for no change */
            if (cistrcmp(cb->f[1], "default") != 0) {
                strncpy(ctlr->essid, cb->f[1], sizeof(ctlr->essid)-1);
                ctlr->essid[sizeof(ctlr->essid)-1] = 0;
            } else {
                memset(ctlr->essid, 0, sizeof(ctlr->essid));
            }
        } else if (ctlr->essid[0] == 0) {
            cmderror(cb, "essid not set");
        }
        if ((i = atoi(cb->f[2])) >= 0 && i <= 16)
            ctlr->chanid = i;
        else
            cmderror(cb, "bad channel number");
        if (ctlr->essid[0])
            wlcreateAP(ctlr, ctlr->essid, ctlr->chanid, atoi(cb->f[3]));
        break;
    case CMdown:
        /* linkdown させる */
        wlcmdint(ctlr, 3, 0);           /* DOWN */
        ctlr->status = Disconnected;
        break;
    }
    poperror();
    kmfree(cb);
    return n;
}

/* ctlr->bssidをbssidにコピーする */
static void etherbcmgetbssid (struct Ether *edev, void *bssid)
{
    Ctlr* ctlr;

    ctlr = edev->ctlr;
    memcpy(bssid, ctlr->bssid, Eaddrlen);
}

/* secsをctlr->scansecsにセットする */
static void etherbcmscan(void *a, uint secs)
{
    Ether* edev;
    Ctlr* ctlr;

    edev = a;
    ctlr = edev->ctlr;
    ctlr->scansecs = secs;
}

/* 登録されているイベントハンドラをtype, paramsを引数に実行する */
static void callevhndlr(Ctlr* ctlr, ether_event_type_t type, const ether_event_params_t *params)
{
    if (ctlr->evhndlr != 0)
        (*ctlr->evhndlr)(type, params, ctlr->evcontext);
}

/* イベントハンドラhndlr、イベントコンテキストcontextを登録する */
static void etherbcmsetevhndlr(struct Ether *edev, ether_event_handler_t *hndlr, void *context)
{
    Ctlr* ctlr;

    ctlr = edev->ctlr;
    ctlr->evcontext = context;
    ctlr->evhndlr = hndlr;
}

/* イーサネットデバイスをattachする */
static void etherbcmattach(Ether *edev)
{
    Ctlr *ctlr;

    ctlr = edev->ctlr;
    qlock(&ctlr->alock);
    if (waserror()) {
        print("ether4330: attach failed: %s\n", up->errstr);
        qunlock(&ctlr->alock);
        nexterror();
    }
    /* edevがセットされていない場合: 初期化が行われていない */
    if (ctlr->edev == nil) {
        /* 1. sdio, sbが初期化されていない場合は初期化する */
        if (ctlr->chipid == 0) {
            sdioinit();
            sbinit(ctlr);
        }
        /* 2. ファームウェアをロードする */
        fwload(ctlr);
        /* 3. sbを有効にする */
        sbenable(ctlr);
        /* 4. 受信処理を行うrproc()を実行するカーネルプロセスを作成する */
        kthread_create("wifireader", rproc, edev, 32);
        /* 5. スキャン処理を行うlproc()を実行するカーネルプロセスを作成する */
        kthread_create("wifitimer", lproc, edev, 32);
        /* 6. 規制ファイルが存在する場合はロードする */
        if (ctlr->regufile)
            reguload(ctlr, ctlr->regufile);     // brcmfmac43455-sdio.clm_blob
            //reguload2(ctlr);
        /* 7. wlanを初期化する */
        wlinit(edev, ctlr);
        /* 8. edevをセットする */
        ctlr->edev = edev;
    }
    qunlock(&ctlr->alock);
    poperror();
}

/* bufの内容に基づいてマルチキャストをセットする */
static void ethersetmulticast(Ether *edev, void *buf, long n)
{
    Ctlr *ctlr;

    ctlr = edev->ctlr;
    /* 1. 変数 mcast_list (マルチキャストリスト) をセットする */
    wlsetvar(ctlr, "mcast_list", buf, n);
    /* 2. 変数 allmulti をリセットする */
    wlsetint(ctlr, "allmulti", 0);
}


/* wlanを停止する */
static void etherbcmshutdown(Ether *edev)
{
    Ctlr *ctlr;

    ctlr = edev->ctlr;
    qlock(&ctlr->alock);
    wlcmdint(ctlr, 3, 0);        /* DOWN */
    qunlock(&ctlr->alock);

    sdioreset();
}

/* イーサネットドライバとして登録するpnpハンドラ関数 */
static int etherbcmpnp(Ether *edev)
{
    Ctlr *ctlr;

    ctlr = kmzalloc(sizeof(Ctlr));
    //memset(ctlr, 0, sizeof(Ctlr));
    ctlr->chanid = Wifichan;
    initlock(&ctlr->txwinlock, "txwin");
    edev->ctlr = ctlr;
    edev->attach = etherbcmattach;
    edev->transmit = etherbcmtransmit;
    edev->ifstat = etherbcmifstat;
    edev->ctl = etherbcmctl;
    edev->getbssid = etherbcmgetbssid;
    edev->scanbs = etherbcmscan;
    edev->setevhndlr = etherbcmsetevhndlr;
    edev->setmulticast = ethersetmulticast;
    edev->shutdown = etherbcmshutdown;
    edev->arg = edev;

    return 0;
}

/* WiFiデバイス(CYW43XX)を"4330"の名前で登録する */
void ether4330link(void)
{
    addethercard("4330", etherbcmpnp);
}
