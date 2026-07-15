#ifndef H_INC_WLAN_P9ARCH_H
#define H_INC_WLAN_P9ARCH_H

#include <types.h>
#include <gpio.h>
#include <irq.h>
#include <p9util.h>
#include <bcm4343.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_IOMEM_AXI_START     0x1000000000UL        // AXI peripherals
#define VIRTIO                  MEM_IOMEM_AXI_START

enum gpio_mode_t {
    Alt0 = 4,
    Alt1 = 5,
    Alt2 = 6,
    Alt3 = 7,
    Alt4 = 3,
    Alt5 = 2
};

void gpiosel(unsigned pin, enum gpio_mode_t mode);

void gpiopulloff(unsigned pin);
void gpiopullup(unsigned pin);

unsigned getclkrate(unsigned clk);
#define ClkEmmc         1

#define delay           __p9delay
#define microdelay      __p9microdelay
void delay(unsigned msecs);
void microdelay(unsigned usecs);

void coherence(void);

typedef uint32_t        Ureg;
typedef void irqhandler_t(Ureg *, void *context);
void intrenable(unsigned irq, irqhandler_t *handler, void *context, unsigned, const char *name);

#define IRQmmc        IRQ_ARASANSDIO      // 62

void dmastart(unsigned chan, unsigned dev, unsigned dir, void *from, void *to, size_t len);
int dmawait(unsigned chan);
#define DmaChanEmmc     0       // unused
#define DmaDevEmmc      11      // emmcのdreq番号
#define DmaM2D          0       // master -> device
#define DmaD2M          1       // device -> master

void cachedinvse(void *buf, size_t len);

#define HZ        100

void p9arch_init(void);

#ifdef __cplusplus
}
#endif

#endif
