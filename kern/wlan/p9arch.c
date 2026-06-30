#include <types.h>
#include <wlan/p9arch.h>
#include <mbox.h>
#include <arm.h>
#include <dma.h>
#include <gpio.h>
#include <irq.h>

static struct CDMAChannel *dma_chan;            // DMAチャンネル

// pinの機能をmodeにセットする
void gpiosel(unsigned pin, enum gpio_mode_t mode)
{
    uintptr_t nSelReg = GPFSEL0 + (pin / 10) * 4;
    unsigned nShift = (pin % 10) * 3;

    //PeripheralEntry ();

    uint32_t nValue = get32(nSelReg);
    nValue &= ~(7 << nShift);
    nValue |= mode << nShift;
    put32(nSelReg, nValue);

    //PeripheralExit ();
}

// pinのpull up/down抵抗をセットする
static void gpiopull(unsigned pin, unsigned mode)
{
    //PeripheralEntry ();

    uint32_t nRegOffset = (pin / 32) * 4;
    uint32_t nRegMask = 1 << (pin % 32);

    // See: https://forums.raspberrypi.com/viewtopic.php?t=163352#p1059178
    uintptr_t nClkReg = GPPUDCLK0 + nRegOffset;

    put32(GPPUD, mode);
    delayus(5);        // 1us should be enough, but to be sure
    put32(nClkReg, nRegMask);
    delayus(5);        // 1us should be enough, but to be sure
    put32(GPPUD, 0);
    put32(nClkReg, 0);

    //PeripheralExit ();
}

// pinをpull up/down抵抗をオフにする
void gpiopulloff (unsigned pin)
{
    gpiopull (pin, 0);
}

// pinをpull up抵抗をオンにする
void gpiopullup (unsigned pin)
{
    gpiopull (pin, 2);
}


// クロックID clkのクロックレートを取得する
unsigned getclkrate(unsigned clk)
{
    return mbox_get_clock_rate(clk);
}

// msecs ミリ秒 遅延させる
void delay(unsigned msecs)
{
    delayus(msecs * 1000);
}

// usecs マイクロ秒 遅延させる
void microdelay(unsigned usecs)
{
    delayus(usecs);
}

// データバリアを実行する
void coherence(void)
{
    dsb();
}

static irqhandler_t *s_pIRQHandler;

// IRQハンドラのスタブ
static void IRQStub(void *context)
{
    (*s_pIRQHandler)(0, context);
}

// 割り込み番号 irq のIRQを有効にする
void intrenable (unsigned irq, irqhandler_t *handler, void *context, unsigned, const char *name)
{
    s_pIRQHandler = handler;
    irq_register(irq, IRQStub, context);
}

// デバイスとのDMA転送を開始する
void dmastart(unsigned chan, unsigned dev, unsigned dir, void *from, void *to, size_t len)
{
    // デバイスからマスタにデータを転送
    if (dir == DmaD2M) {
        CDMAChannelSetupIORead(dma_chan, to, (uint32_t) (uintptr_t) from, len, (TDREQ) dev);
    } else {
        CDMAChannelSetupIOWrite(dma_chan, (uint32_t) (uintptr_t) to, from, len, (TDREQ) dev);
    }

    CDMAChannelStart(dma_chan);
}

// DMA転送の完了を待機する
int dmawait (unsigned chan)
{
    if (!CDMAChannelWait(dma_chan)) {
        return -1;
    }

    return 0;
}


// データキャッシュを破棄する
void cachedinvse (void *buf, size_t len)
{
    dccivac(buf, (int)len);
}

// p9アーキテクチャを初期化する
void p9arch_init()
{
    // DMA転送用のチャネルをセット : TODO dma_channel を作成する
    dma_chan = dma_init(DMA_CHANNEL_NORMAL);
    assert (dma_chan != 0);
}
