//
// usbendpoint.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright(C) 2014  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <usb/usb_endpoint.h>
#include <types.h>
#include <console.h>
#include <slab.h>
#include <string.h>

struct slab_cache *ENDPOINT;

void usb_endpoint_init(void)
{
    ENDPOINT = slab_cache_create("urb", sizeof(usb_request_t), 64);
}

usb_endpoint_t *usb_endpoint_alloc(usb_dev_t *dev, const usb_ep_desc_t *desc)
{
    usb_endpoint_t *ep = (usb_endpoint_t *)slab_cache_alloc(ENDPOINT);
    memset(ep, 0, sizeof(usb_request_t));
    ep->dev = dev;
    ep->interval = 1;
    if (desc != 0) {
        ep->nextpid = usb_pid_data0;
        assert(desc->length >= sizeof *desc); // クラス固有トレイラがある場合があるので>=
        assert(desc->type == DESCRIPTOR_ENDPOINT);

        switch(desc->attr & 0x03) {
        case 2:
            ep->type = ep_type_bulk;
            break;

        case 3:
            ep->type = ep_type_interrupt;
            break;

        default:
            assert(0);      // アイソクロナスは未実装。エンドポイント
            return NULL;    // コンフィグレーションは属性クラスのドライバでチェックする
        }

        ep->num       = desc->addr & 0x0F;
        ep->in        = desc->addr & 0x80 ? true : false;
        ep->xsize     = desc->xsize & 0x7FF;

        // インタラプト転送
        if (ep->type == ep_type_interrupt) {
            uint8_t interval = desc->interval;
            if (interval < 1) {
                interval = 1;
            }

            // see USB 2.0 spec chapter 9.6.6
            if (ep->dev->speed < usb_speed_high) {    // LS/FS
                ep->interval = interval;
            } else {                                    // HS
                if (interval > 16) {
                    interval = 16;
                }
                unsigned value = 1 << (interval - 1);
                ep->interval = value / 8;
                if(ep->interval < 1) {
                    ep->interval = 1;
                }
            }
#ifndef USE_USB_SOF_INTR
            // interval 20ms is minimum to reduce interrupt rate
            if (ep->interval < 20)
            {
                ep->interval = 20;
            }
#endif
        }

        // バルクエンドポイントでLPの場合の回避策、通常、仕様では禁止されている
        if (ep->dev->speed == usb_speed_low && ep->type == ep_type_bulk) {
            warn("Device is not fully USB compliant");
            // 割り込みEPにする
            ep->type = ep_type_interrupt;
            // 最大パケットサイズは8以下
            if (ep->xsize > 8)
                ep->xsize = 8;

#ifdef USE_USB_SOF_INTR
            ep->interval = 1;
#else
            ep->interval = 20;
#endif
        }
    } else {
        ep->num = 0;
        ep->type = ep_type_control;
        ep->in = false;
        ep->xsize = USB_DEFAULT_MAX_PACKET_SIZE;
        ep->interval = 1;
        ep->nextpid = usb_pid_setup;
    }
    return ep;
}

void usb_endpoint_free(usb_endpoint_t *self)
{
    slab_cache_free(ENDPOINT, self);
}

usb_pid_t usb_endpoint_get_nextpid(usb_endpoint_t *self, boolean ststatus)
{
    if (ststatus) {
        assert(self->type == ep_type_control);
        return usb_pid_data1;
    }

    trace("nextpid: %d", self->nextpid);
    return self->nextpid;
}

void usb_endpoint_skip_pid(usb_endpoint_t *self, unsigned packets, boolean ststatus)
{
    if (!ststatus) {
        switch(self->nextpid) {
        case usb_pid_setup:
            self->nextpid = usb_pid_data1;
            break;

        case usb_pid_data0:
            if (packets & 1)
                self->nextpid = usb_pid_data1;
            break;

        case usb_pid_data1:
            if (packets & 1)
                self->nextpid = usb_pid_data0;
            break;

        default:
            info("bad next pid: %d", self->nextpid);
            assert(0);
            break;
        }
    } else {
        if (self->type != ep_type_control) warn("type = %d", self->type);
        //assert(self->type == ep_type_control);
        self->nextpid = usb_pid_setup;
    }
}

void usb_endpoint_reset_pid(usb_endpoint_t *self)
{
    assert(self->type == ep_type_control || self->type == ep_type_bulk);
    self->nextpid = (self->type == ep_type_control) ? usb_pid_setup : usb_pid_data0;
}

void usb_endpoint_debug(usb_endpoint_t *self)
{
    cprintf("usb_ep [0x%p] usb_dev: 0x%p, num: %u, type: %d, dir: %s, size: %u, interval: %u: next: %d\n", self, self->dev, self->num, self->type, self->in ? "in " : "out", self->xsize, self->interval, self->nextpid);
}
