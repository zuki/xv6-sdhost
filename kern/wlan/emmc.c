/*
 * bcm2835 external mass media controller (mmc / sd host interface)
 *
 * Copyright � 2012 Richard Miller <r.miller@acm.org>
 */

/*
    公式には記載されていないがemmcは別のGPIOピンに接続できる
        48-53 (SD card)             // ALT3のdefault : ALT0 で SDHOST経由でSDカードに接続
        22-27 (P1 header)
        34-39 (wifi - pi3 only)     // ALT3 で SDIO経由でwifi chip (cyw43455)に接続
    ALT3機能を使用して、必要なルーティングを有効にする
 */

#include <wlan/p9compat.h>
#include <console.h>
#include <mm.h>
#include <string.h>
#include <spinlock.h>

extern long get_ticks(void);

// EMMCの基底アドレス (raspi3b+: 0x3F30_0000)
#define EMMCREGS    (VIRTIO+0x300000)


enum {
    Extfreq         = 100*Mhz,  /* 推定外部クロック周波数(vcoreから取得できない場合） */
    Initfreq        = 400000,   /* MMCの初期化時周波数 (400KHz) */
    SDfreq          = 25*Mhz,   /* 標準SD周波数 */
    SDfreqhs        = 50*Mhz,   /* 高速(HS)周波数 */
    DTO             = 14,       /* データタイムアウト指数 (guesswork) */

    // コマンドインデックス
    GoIdle          = 0,        /* mmc/sdio GO_IDLE_STATE (CMD0) */
    MMCSelect       = 7,        /* mmc/sdio SELECT/DESELECT_CARD (CMD7) */
    Setbuswidth     = 6,        /* mmc/sd set bus width command (ACMD6) */
    Switchfunc      = 6,        /* mmc/sd SWITCH_FUNC (CMD6) */
    Voltageswitch   = 11,       /* md/sdio VOLTAGE_SWITCH (CMD11) 1.8Vに切り替え */
    IORWdirect      = 52,       /* sdio IO_RW_DIRECT (CMD52) */
    IORWextended    = 53,       /* sdio IO_RW_EXTENDED (CMD53) */
    Appcmd          = 55,       /* mmc/sd APP_CMD (CMD55) 次のコマンドはACMD */
};

enum {
    /* EMMCレジスタ: アドレス >> 2 = インデックス */
    Arg2            = 0x00>>2,      // 0 : ARG2
    Blksizecnt      = 0x04>>2,      // 1 : BLKSIZECNT
    Arg1            = 0x08>>2,
    Cmdtm           = 0x0c>>2,
    Resp0           = 0x10>>2,
    Resp1           = 0x14>>2,
    Resp2           = 0x18>>2,
    Resp3           = 0x1c>>2,
    Data            = 0x20>>2,
    Status          = 0x24>>2,
    Control0        = 0x28>>2,
    Control1        = 0x2c>>2,
    Interrupt       = 0x30>>2,      // 12 : INTERRUPT
    Irptmask        = 0x34>>2,      // 13 : IRPT_MASK
    Irpten          = 0x38>>2,      // 14 : IRPT_EN
    Control2        = 0x3c>>2,      // 15 : CONTROL2
    Forceirpt       = 0x50>>2,      // 20 : FORCE_IRPT
    Boottimeout     = 0x70>>2,      // 28 : BOOT_TIMEOUT
    Dbgsel          = 0x74>>2,      // 29 : DBG_SEL
    Exrdfifocfg     = 0x80>>2,      // 32 : EXRDFIFO_CFG
    Exrdfifoen      = 0x84>>2,      // 31 : EXRDFIFO_EN
    Tunestep        = 0x88>>2,      // 32 : TUNE_STEP
    Tunestepsstd    = 0x8c>>2,      // 33 : TUNE_STEPS_STD
    Tunestepsddr    = 0x90>>2,      // 34 : TUNE_STEPS_DDR
    Spiintspt       = 0xf0>>2,      // 60 : SPI_INT_SPT SPI
    Slotisrver      = 0xfc>>2,      // 63 : SLOTISR_VER Slot

    /* Control0: 0x28 */
    Hispeed         = 1<<2,         /* 高速モードを選択 */
    Dwidth4         = 1<<1,         /* 4データラインを使用 */
    Dwidth1         = 0<<1,         /* 1データラインを使用 */

    /* Control1: 0x2C */
    Srstdata        = 1<<26,        /* reset data circuit */
    Srstcmd         = 1<<25,        /* reset command circuit */
    Srsthc          = 1<<24,        /* reset complete host controller */
    Datatoshift     = 16,           /* data timeout unit exponet: bit shift */
    Datatomask      = 0xF0000,      /*   DATA_TIUNIT[19:16]: bit mask */
    Clkfreq8shift   = 8,            /* SD clock base divider LSBs: bit shift */
    Clkfreq8mask    = 0xFF00,       /*   CLK_FREQ8[19:16]: bit mask */
    Clkfreqms2shift = 6,            /* SD clock base divider MSBs: bit shift*/
    Clkfreqms2mask  = 0xC0,         /*   CLK_FREQ_MS2[7:6]: bit mask */
    Clkgendiv       = 0<<5,         /* CLK_GENSEL: SD clock divided */
    Clkgenprog      = 1<<5,         /*             SD clock programmable */
    Clken           = 1<<2,         /* CLK_EN: SD clock enable */
    Clkstable       = 1<<1,         /* CLK_STABLE: sd clock is stable */
    Clkintlen       = 1<<0,         /* CLK_INTLEN: enable internal EMMC clocks */

    /* CMDTM: 0xC */
    Indexshift      = 24,           /* CMD_INDEXビットのシフト値*/
    Suspend         = 1<<22,        /* CMD_TYPE[23:22]: suspend */
    Resume          = 2<<22,        /*                  resume  */
    Abort           = 3<<22,        /*                  abort   */
    Isdata          = 1<<21,        /* CMD_ISDATA: データ転送あり */
    Ixchken         = 1<<20,        /* CMD_IXCHK_EN: 応答のcmd_idxをチェック */
    Crcchken        = 1<<19,        /* CMD_CRCCHK_EN: 応答のCRCをチェック */
    Respmask        = 3<<16,        /* CMD_RSPNS_TYPE[16:17] マスク      */
    Respnone        = 0<<16,        /*                 応答なし          */
    Resp136         = 1<<16,        /*                 136bit 応答       */
    Resp48          = 2<<16,        /*                 48bit 応答        */
    Resp48busy      = 3<<16,        /*                 48bit 応答 + busy */
    Multiblock      = 1<<5,         /* TM_MULTI_BLOCK: マルチブロック転送 */
    Host2card       = 0<<4,         /* TM_DAT_DIR: ホストからカードへ転送 */
    Card2host       = 1<<4,         /* TM_DAT_DIR: カードからホストへ転送 */
    Autocmd12       = 1<<2,         /* TM_AUTO_CMD_EN: データ転送後にCMD12を送信 */
    Autocmd23       = 2<<2,         /*                 データ転送後にCMD23を送信*/
    Blkcnten        = 1<<1,         /* TM_BLKCNT_EN: マルチブロック転送のブロックカウンタを有効化 */

    /* Interrupt: flags: 0x30, mask: 0x34, enable: 0x38 : 3レジスタ共通 */
    Acmderr         = 1<<24,
    Denderr         = 1<<22,
    Dcrcerr         = 1<<21,
    Dtoerr          = 1<<20,
    Cbaderr         = 1<<19,
    Cenderr         = 1<<18,
    Ccrcerr         = 1<<17,
    Ctoerr          = 1<<16,
    Err             = 1<<15,
    Cardintr        = 1<<8,
    Cardinsert      = 1<<6,         /* not in Broadcom datasheet */
    Readrdy         = 1<<5,
    Writerdy        = 1<<4,
    Datadone        = 1<<1,
    Cmddone         = 1<<0,

    /* Status: 0x24 */
    Bufread         = 1<<11,        /* not in Broadcom datasheet */
    Bufwrite        = 1<<10,        /* not in Broadcom datasheet */
    Readtrans       = 1<<9,         /* EMMCから新規データ読み込みOK */
    Writetrans      = 1<<8,         /* EMMCへの新規データ書き込みOK */
    Datactive       = 1<<2,         /* 少なくとも1本データラインがactive */
    Datinhibit      = 1<<1,         /* データラインは使用中 */
    Cmdinhibit      = 1<<0,         /* コマンドラインは使用中 */
};

// コマンドの付帯情報: 応答の種類、CRCのチェックなど（idxはコマンド番号）
static int cmdinfo[64] = {
    [0]  Ixchken,
    [2]  Resp136,
    [3]  Resp48 | Ixchken | Crcchken,
    [5]  Resp48,
    [6]  Resp48 | Ixchken | Crcchken,
    [7]  Resp48busy | Ixchken | Crcchken,
    [8]  Resp48 | Ixchken | Crcchken,
    [9]  Resp136,
    [11] Resp48 | Ixchken | Crcchken,
    [12] Resp48busy | Ixchken | Crcchken,
    [13] Resp48 | Ixchken | Crcchken,
    [16] Resp48,
    [17] Resp48 | Isdata | Card2host | Ixchken | Crcchken,
    [18] Resp48 | Isdata | Card2host | Multiblock | Blkcnten | Ixchken | Crcchken,
    [24] Resp48 | Isdata | Host2card | Ixchken | Crcchken,
    [25] Resp48 | Isdata | Host2card | Multiblock | Blkcnten | Ixchken | Crcchken,
    [41] Resp48,
    [52] Resp48 | Ixchken | Crcchken,
    [53] Resp48    | Ixchken | Crcchken | Isdata,
    [55] Resp48 | Ixchken | Crcchken,
};

typedef struct Ctlr Ctlr;

// EMMCコントロール構造体
struct Ctlr {
    Rendez      r;
    Rendez      cardr;
    int         fastclock;
    uint64_t    extclk;
    int         appcmd;
    struct spinlock lock;
    uint8_t *   dmabuf;
#define DMABUFSZ        (4096)
#define CACHELINESZ     (64)
};

static Ctlr emmc;

static void mmcinterrupt(Ureg*, void*);

// 指定のレジスタregに値valを書き込む
static void WR(int reg, uint32_t val)
{
    volatile uint32_t *r = (uint32_t*)EMMCREGS;

    if (0)debug("WR %2.2x %x\n", reg<<2, val);
    microdelay(emmc.fastclock ? 2 : 20);
    coherence();    // データバリア
    r[reg] = val;
}

// SDクロックの分周比をCONTROL1レジスタのCLK_FREQ8とCLK_FREQ_MS2にセットする
static uint clkdiv(uint d)
{
    uint v;

    assert(d < 1<<10);
    v = (d << Clkfreq8shift) & Clkfreq8mask;
    v |= ((d >> 8) << Clkfreqms2shift) & Clkfreqms2mask;
    return v;
}

// SDクロックをセットする
static void emmcclk(uint freq)
{
    volatile uint32_t *r;
    uint div;
    int i;

    r = (uint32_t*)EMMCREGS;
    div = emmc.extclk / (freq<<1);
    if (emmc.extclk / (div<<1) > freq)
        div++;
    // CONTROL1レジスタに書き込み
    WR(Control1, clkdiv(div) |
        DTO<<Datatoshift | Clkgendiv | Clken | Clkintlen);
    // SDクロックが安定するのを待つ
    for(i = 0; i < 1000; i++) {
        delay(1);
        if (r[Control1] & Clkstable)
            break;
    }
    if (i == 1000)
        warn("emmc: can't set clock to %u", freq);
}


// sleepから起床時に呼び出されるハンドラ
static int datadone(void*dummy)
{
    int i;

    volatile uint32_t *r = (uint32_t*)EMMCREGS;
    i = r[Interrupt];
    return i & (Datadone|Err);
}

// カード割り込みがあった
static int cardintready(void *dummy)
{
    int i;

    volatile uint32_t *r = (uint32_t*)EMMCREGS;
    i = r[Interrupt];
    return i & Cardintr;
}

// EMMCの初期化
static int emmcinit(void)
{
    volatile uint32_t *r;
    uint64_t clk;

    // DMAバッファを確保
    emmc.dmabuf = kmalloc(DMABUFSZ);
    assert(emmc.dmabuf);

    initlock(&emmc.lock, "emmc");

    // EMMCのクロックレートをmbox経由 (tag: 0x00030002) で取得
    clk = getclkrate(ClkEmmc);
    if (clk == 0) {
        clk = Extfreq;
        debug("emmc: assuming external clock %lud Mhz\n", clk/1000000);
    }
    emmc.extclk = clk;
    r = (uint32_t*)EMMCREGS;
    if (0)debug("emmc control %8.8x %8.8x %8.8x\n",
        r[Control0], r[Control1], r[Control2]);
    // ホスト回路を完全にリセット
    WR(Control1, Srsthc);
    delay(10);
    while(r[Control1] & Srsthc)
        ;
    // データ処理回路をリセット
    WR(Control1, Srstdata);
    delay(10);
    // CONTROL1レジスタを0クリア
    WR(Control1, 0);

    return 0;
}

// ホストコントローラのバージョン問い合わせ
static int emmcinquiry(char *inquiry, int inqlen)
{
    volatile uint32_t *r;
    uint ver;

    r = (uint32_t*)EMMCREGS;
    // ver[31:24]: ベンダーバージョン, [23:16]: ホストコントローラ仕様バージョン
    ver = r[Slotisrver] >> 16;
    return snprintf(inquiry, inqlen,
        "Arasan eMMC SD Host Controller %2.2x Version %2.2x",
        ver&0xFF, ver>>8);
}

// EMMCを有効にする
static void emmcenable(void)
{
    emmcclk(Initfreq);        // emmcクロック設定
    WR(Irpten, 0);            // 割り込みをすべて無効に
    WR(Irptmask, ~0);        // 割り込みをすべてマスク
    WR(Interrupt, ~0);        // 割り込みをすべてクリア
    intrenable(IRQmmc, mmcinterrupt, nil, 0, "mmc");    // IRQ=62を有効化
}

// カード割り込みがかかるのを待つ(割り込みフラグの内容を返す)
int sdiocardintr(int wait)
{
    volatile uint32_t *r;
    int i;

    r = (uint32_t*)EMMCREGS;
    WR(Interrupt, Cardintr);    // カード割り込みフラグをクリア
    while (((i = r[Interrupt]) & Cardintr) == 0) {
        if (!wait)
            return 0;
        WR(Irpten, r[Irpten] | Cardintr);        // カード割り込みを有効にする
        p9sleep(&emmc.cardr, &emmc.lock, cardintready, 0);    // カード割り込みを待ってsleep
    }
    WR(Interrupt, Cardintr);    // カード割り込みフラグをクリア
    return i;
}

// コマンドcmdを引数argで送信し、応答をrespにセットする
static int emmccmd(uint32_t cmd, uint32_t arg, uint32_t *resp)
{
    volatile uint32_t *r;
    uint32_t c;
    int i;
    uint64_t now;

    r = (uint32_t*)EMMCREGS;
    assert(cmd < nelem(cmdinfo) && cmdinfo[cmd] != 0);
    // CMDTM(0xc)の値をセット
    c = (cmd << Indexshift) | cmdinfo[cmd];
    /*
     * CMD6 may be Setbuswidth or Switchfunc depending on Appcmd prefix
     *  CMD6: SWITCH_FUNC, ACMD6: SET_BUS_WIDTH
     */
    if (cmd == Switchfunc && !emmc.appcmd)
        c |= Isdata|Card2host;
    // CMD53
    if (cmd == IORWextended) {
        // write
        if (arg & (1<<31))
            c |= Host2card;
        // read
        else
            c |= Card2host;
        // マルチブロック転送
        if ((r[Blksizecnt]&0xFFFF0000) != 0x10000)
            c |= Multiblock | Blkcnten;
    }
    /*
     * GoIdle は新しいカードの挿入を示すのでバス幅と速度をリセットする
     */
    if (cmd == GoIdle) {
        WR(Control0, r[Control0] & ~(Dwidth4|Hispeed));
        emmcclk(Initfreq);
    }
    // コマンドラインが使用中: コマンド処理回路をリセット
    if (r[Status] & Cmdinhibit) {
        print("emmccmd: need to reset Cmdinhibit intr %x stat %x\n",
            r[Interrupt], r[Status]);
        WR(Control1, r[Control1] | Srstcmd);    // コマンド処理回路をリセット
        while(r[Control1] & Srstcmd)        // コマンド処理回路のリセットを待つ
            ;
        while(r[Status] & Cmdinhibit)        // コマンドラインがあくのを待つ
            ;
    }
    // データラインが使用中: データ処理回路をリセット
    if ((r[Status] & Datinhibit) &&
       ((c & Isdata) || (c & Respmask) == Resp48busy)) {
        print("emmccmd: need to reset Datinhibit intr %x stat %x\n",
            r[Interrupt], r[Status]);
        WR(Control1, r[Control1] | Srstdata);
        while(r[Control1] & Srstdata)
            ;
        while(r[Status] & Datinhibit)
            ;
    }
    // 引数をレジスタにセット
    WR(Arg1, arg);
    // カード割り込み以外の割り込みあり
    if ((i = (r[Interrupt] & ~Cardintr)) != 0) {
        // カード挿入割り込み以外の割り込み -> クリア
        if (i != Cardinsert)
            print("emmc: before command, intr was %x\n", i);
        WR(Interrupt, i);
    }
    // コマンドレジスタに書き込んでコマンド実行
    WR(Cmdtm, c);
    now = get_ticks();
    // 処理が完了（エラー発生を含む）するのを待つ（タイムアウト1秒）
    while(((i=r[Interrupt])&(Cmddone|Err)) == 0)
        if (get_ticks() - now > HZ)
            break;
    // エラー発生
    if ((i&(Cmddone|Err)) != Cmddone) {
        // タイムアウトエラー以外のエラーが発生
        if ((i&~(Err|Cardintr)) != Ctoerr)
            print("emmc: cmd %x arg %x error intr %x stat %x\n", c, arg, i, r[Status]);
        // 割り込みフラグをクリア
        WR(Interrupt, i);
        // コマンドラインをあける
        if (r[Status]&Cmdinhibit) {
            WR(Control1, r[Control1]|Srstcmd);
            while(r[Control1]&Srstcmd)
                ;
        }
        // エラーを投げる
        p9error(Eio);
    }
    // 割り込みフラグでもはや不要なフラグをクリア
    WR(Interrupt, i & ~(Datadone|Readrdy|Writerdy));
    // 応答をセット
    switch(c & Respmask) {
    case Resp136:
        resp[0] = r[Resp0]<<8;
        resp[1] = r[Resp0]>>24 | r[Resp1]<<8;
        resp[2] = r[Resp1]>>24 | r[Resp2]<<8;
        resp[3] = r[Resp2]>>24 | r[Resp3]<<8;
        break;
    case Resp48:
    case Resp48busy:
        resp[0] = r[Resp0];
        break;
    case Respnone:
        resp[0] = 0;
        break;
    }
    // ビジーシグナルありの場合、データ転送の完了をsleepして待機
    if ((c & Respmask) == Resp48busy) {
        // エラービットとデータ転送完了フラグをクリア
        WR(Irpten, r[Irpten]|Datadone|Err);
        // データ転送が完了するまでsleep
        p9tsleep(&emmc.r, &emmc.lock, datadone, 0, 3000);
        i = r[Interrupt];
        if ((i & Datadone) == 0)
            print("emmcio: no Datadone after CMD%d\n", cmd);
        if (i & Err)
            print("emmcio: CMD%d error interrupt %x\n",
                cmd, r[Interrupt]);
        WR(Interrupt, i);
    }
    /*
     * CMD7の場合、より早いクロックを使用する
     */
    if (cmd == MMCSelect) {
        delay(1);
        emmcclk(SDfreq);    // 標準SD周波数
        delay(1);
        emmc.fastclock = 1;
    }
    if (cmd == Setbuswidth) {
        // ACMD6: SET_BUS_WIDTH の場合
        if (emmc.appcmd) {
            /*
             * If card bus width changes, change host bus width
             */
            switch(arg) {
            case 0:    // 1bit データラインを使用
                WR(Control0, r[Control0] & ~Dwidth4);
                break;
            case 2:    // 4bit データラインを使用
                WR(Control0, r[Control0] | Dwidth4);
                break;
            default:
                break;
            }
        // CMD6: SWITCH_FUNC の場合
        } else {
            /*
             * If card switched into high speed mode, increase clock speed
             */
            if ((arg&0x8000000F) == 0x80000001) {
                delay(1);
                emmcclk(SDfreqhs);
                delay(1);
            }
        }
    // CMD52で書き込み、FN0, レジスタ0x7: Bus Interface Control
    } else if (cmd == IORWdirect && (arg & ~0xFF) == (1<<31|0<<28|7<<9)) {
        switch(arg & 0x3) {
        case 0:
            WR(Control0, r[Control0] & ~Dwidth4);    // バス幅 1bit
            break;
        case 2:
            WR(Control0, r[Control0] | Dwidth4);    // バス幅 4bit
            //WR(Control0, r[Control0] | Hispeed);
            break;
        }
    }
    emmc.appcmd = (cmd == Appcmd);
    return 0;
}

// io時のブロックサイズとブロック数をセット
static void
emmciosetup(int write, void *buf, int bsize, int bcount)
{
    USED(write);
    USED(buf);
    // BLKSIZECNTアドレスに書き込み
    WR(Blksizecnt, bcount<<16 | bsize);
}

// データ転送
static void
emmcio(int write, uchar *buf, int len)
{
    volatile uint32_t *r;
    uchar *dmabuf = 0;
    int i;

    r = (uint32_t*)EMMCREGS;
    assert((len&3) == 0);
    okay(1);
    if (waserror()) {
        okay(0);
        nexterror();
    }
    // DMA経由で読み書きする
    // bufがキャッシュサイズにアラインしており、lenがキャッシュサイズ以上の
    // 場合は、dmaバッファとしてbufをそのまま、それ以外はemmc.dmabufを使う
    if (((unsigned long)buf & (CACHELINESZ-1)) ||
        (len & (CACHELINESZ-1))) {
        assert(len <= DMABUFSZ);
        dmabuf = emmc.dmabuf;
    }
    if (write) {
        // dmabufを使う場合はbufからdmabufにデータをセット
        if (dmabuf)
            memcpy(dmabuf, buf, len);
        dmastart(DmaChanEmmc, DmaDevEmmc, DmaM2D,
            dmabuf ? dmabuf : buf, (void *) &r[Data], len);
    }else
        dmastart(DmaChanEmmc, DmaDevEmmc, DmaD2M,
            (void *) &r[Data], dmabuf ? dmabuf : buf, len);
    // DMA転送を待機し、エラーがあった場合はEioエラーを投げる
    if (dmawait(DmaChanEmmc) < 0) {
        error("dmawait failed");
        p9error(Eio);
    }
    if (!write) {
        cachedinvse(dmabuf ? dmabuf : buf, len);
        if (dmabuf)
            memcpy(buf, dmabuf, len);
    }

    // EMMC IRPT_EN: 割り込みを有効化
    WR(Irpten, r[Irpten]|Datadone|Err);
    // 割り込みがかかるまでsleep
    p9tsleep(&emmc.r, &emmc.lock, datadone, 0, 3000);
    // Cardからの割り込み以外の割り込みを取得
    i = r[Interrupt]&~Cardintr;
    // 転送が終了していない(timeout)
    if ((i & Datadone) == 0) {
        print("emmcio: %d timeout intr %x stat %x\n",
            write, i, r[Status]);
        WR(Interrupt, i);
        p9error(Eio);
    }
    // 転送中にエラー発生
    if (i & Err) {
        print("emmcio: %d error intr %x stat %x\n",
            write, r[Interrupt], r[Status]);
        WR(Interrupt, i);
        p9error(Eio);
    }
    // OK: 割り込みフラグをクリア
    if (i)
        WR(Interrupt, i);
    poperror();
    okay(0);
}

// IRQ63の割り込みハンドラ
static void
mmcinterrupt(Ureg*regs, void*param)
{
    volatile uint32_t *r;
    int i;

    r = (uint32_t*)EMMCREGS;
    i = r[Interrupt];
    if (0) debug("mmcinterrupt: intr %x\n", i);
    // 処理終了
    if (i&(Datadone|Err))
        p9wakeup(&emmc.r);
    // カードから割り込み
    if (i&Cardintr)
        p9wakeup(&emmc.cardr);
    // 割り込みクリア
    r[Irpten] &= ~i;
}

// SDio操作関数変数の定義
SDio sdio = {
    "emmc",
    emmcinit,
    emmcenable,
    emmcinquiry,
    emmccmd,
    emmciosetup,
    emmcio,
};
