//
// usbusb_cdcethernet.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2017-2019  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <usb/usb_cdcether.h>
#include <usb/usb.h>
#include <usb/usb_request.h>
#include <usb/usb_string.h>
#include <usb/dwhc_device.h>
#include <usb/usb_dev_ns.h>
#include <types.h>
#include <console.h>
#include <mm.h>
#include <mbox.h>
#include <clock.h>
#include <arm.h>
#include <net/net.h>
#include <net/ether.h>
#include <net/platform.h>
#include <console.h>
#include <string.h>

typedef struct ethernet_functional_descriptor {
    uint8_t      length;
    uint8_t      type;
    uint8_t      subtype;
#define ETHERNET_NETWORKING_FUNCTIONAL_DESCRIPTOR    0x0F
    uint8_t      macaddr;
    uint32_t     statics;
    uint16_t     xsegsize;
    uint16_t     mcfilters;
    uint8_t      powerfilters;
} PACKED eth_func_desc_t;

static boolean usb_cdcether_init_macaddr(usb_cdcether_t *self, uint8_t id);

void usb_cdcether(usb_cdcether_t *self, usb_function_t *func)
{
    usb_function_copy(&self->usb_func, func);
    self->usb_func.configure = usb_cdcether_configure;
    self->bulk_in = 0;
    self->bulk_out = 0;
}

void _usb_cdcether(usb_cdcether_t *self)
{
    if (self->bulk_out != 0) {
        _usb_endpoint(self->bulk_out);
        kmfree(self->bulk_out);
    }

    if (self->bulk_in != 0) {
        _usb_endpoint(self->bulk_in);
        kmfree(self->bulk_in);
    }

    _usb_function(&self->usb_func);
}


boolean usb_cdcether_configure(usb_function_t *func)
{
    usb_cdcether_t *self = (usb_cdcether_t *)func;

    // Ethernetネットワーキング機能ディスクリプタを見つける
    const eth_func_desc_t *eth_desc;
    while ((eth_desc = (eth_func_desc_t *)usb_function_get_desc(&self->usb_func, DESCRIPTOR_CS_INTERFACE)) != 0) {
        if (eth_desc->subtype == ETHERNET_NETWORKING_FUNCTIONAL_DESCRIPTOR)
            break;
    }

    if (eth_desc == 0) {
        error("couldn't find ETHERNET_NETWORKING_FUNCTIONAL_DESCRIPTOR");
        return false;
    }

    // データクラスインタフェースディスクリプタを見つける
    const usb_if_desc_t *if_decs;
    while ((if_decs = (usb_if_desc_t *)usb_function_get_desc(&self->usb_func, DESCRIPTOR_INTERFACE)) != 0) {
        if (if_decs->class    == 0x0A
         && if_decs->subclass == 0x00
         && if_decs->proto    == 0x00
         && if_decs->neps     >= 2) {
            break;
        }
    }

    if (if_decs == 0) {
        error("couldn't find DESCRIPTOR_INTERFACE");
        return false;
    }

    // MACアドレスを初期化
    if (!usb_cdcether_init_macaddr(self, eth_desc->macaddr)) {
        error("Cannot get MAC address");
        return false;
    }

    info("MAC address is %x:%x:%x:%x:%x:%x", self->macaddr[0], self->macaddr[1], self->macaddr[2], self->macaddr[3], self->macaddr[4], self->macaddr[5]);

    // エンドポイントを取得
    const usb_ep_desc_t *ep_desc;
    while ((ep_desc = (usb_ep_desc_t *)usb_function_get_desc(&self->usb_func, DESCRIPTOR_ENDPOINT)) != 0) {
        if ((ep_desc->attr & 0x3F) == 0x02) {       // バルク転送
            if ((ep_desc->addr & 0x80) == 0x80) {   // 入力パイプ
                if (self->bulk_in != 0) {
                    error("bulk_in not null");
                    return false;
                }
                self->bulk_in = (usb_endpoint_t *)kmalloc(sizeof(usb_endpoint_t));
                usb_endpoint2(self->bulk_in, usb_function_get_dev(&self->usb_func), ep_desc);
            } else {                                // 出力パイプ
                if (self->bulk_out != 0) {
                    error("bulk_out not null");
                    return false;
                }
                self->bulk_out = (usb_endpoint_t *)kmalloc(sizeof(usb_endpoint_t));
                usb_endpoint2(self->bulk_out, usb_function_get_dev(&self->usb_func), ep_desc);
            }
        }
    }

    if (self->bulk_in == 0 || self->bulk_out == 0) {
        error("ep bulk_in and bulk_out must exit");
        return false;
    }

    // コンフィグレーションを行う
    if (!usb_function_config(&self->usb_func)) {
        error("config usb_func failed");
        return false;
    }

    // USBデバイスとして登録
    usb_device_ns_add_dev(usb_device_ns_get(), "eth10", self, false);

    // FIXME: ネットデバイスとして登録
    //netdev_add_dev(self->net_dev);

    return true;
}

const char *usb_cdcether_get_macaddr(usb_cdcether_t *self)
{
    return self->macaddr;
}

static ssize_t usb_cdcether_send_frame(struct net_device *dev, const uint8_t *buf, size_t size)
{
    // USB転送（バルク）
    usb_cdcether_t *self = (usb_cdcether_t *)dev->priv;
    return dwhc_xfer(usb_function_get_host(&self->usb_func), self->bulk_out, buf, size);
}

static ssize_t usb_cdcether_receive_frame(struct net_device *dev, uint8_t *buf, size_t size)
{
    usb_request_t urb;
    usb_cdcether_t *self = (usb_cdcether_t *)dev->priv;

    usb_request(&urb, self->bulk_in, buf, FRAME_BUFFER_SIZE, 0);
    urb.onnak = true;

    if (!dwhc_submit_block_request(usb_function_get_host(&self->usb_func), &urb, USB_TIMEOUT_NONE)) {
        error("failed submit block request");
        return -1;
    }

    ssize_t rlen = usb_request_get_resultlen(&urb);
    if (rlen == 0) {
        error("resultlen is 0");
        return -1;
    }

    return rlen;
}

static boolean usb_cdcether_init_macaddr(usb_cdcether_t *self, uint8_t id)
{
    usb_string_t usb_str;
    usb_string(&usb_str, usb_function_get_dev(&self->usb_func));
    if (id == 0 || !usb_string_get_from_desc(&usb_str, id,
            usb_string_get_langid(&usb_str))) {
        error("failed to get lang id");
        return false;
    }

    const char *addr = usb_string_get(&usb_str);

    uint8_t macaddr[MAC_ADDRESS_SIZE];
    for (unsigned i = 0; i < MAC_ADDRESS_SIZE; i++)
    {
        uint8_t byte = 0;
        for (unsigned j = 0; j < 2; j++)
        {
            char c = *addr++;
            if (c > '9') {
                c -= 'A'-'9'-1;
            }
            c -= '0';

            if (!('\0' <= c && c <= '\xF')) {
                return false;
            }

            byte <<= 4;
            byte |= (uint8_t) c;
        }

        macaddr[i] = byte;
    }

    memmove(self->macaddr, macaddr, MAC_ADDRESS_SIZE);

    return true;
}

static int usb_cdcether_net_open(struct net_device *dev)
{
    return 0;
}

static int usb_cdcether_net_close(struct net_device *dev)
{
    return 0;
}

static int usb_cdcether_net_transmit(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst)
{
    return ether_transmit_helper(dev, type, data, len, dst, usb_cdcether_send_frame);
}

static int usb_cdcether_is_linkup(struct net_device *dev)
{
    return 1;
}

struct net_device_ops usb_cdcether_net_ops = {
    .open = usb_cdcether_net_open,
    .close = usb_cdcether_net_close,
    .transmit = usb_cdcether_net_transmit,
    .linkup = usb_cdcether_is_linkup,
};

int usb_cdcether_net_init(usb_cdcether_t *self)
{
    struct net_device *dev;

        // setup device driver structure
    dev = net_device_alloc();
    if (!dev) {
        error("net_device_alloc() failure");
        return -1;
    }
    ether_setup_helper(dev);

    memcpy(dev->addr, self->macaddr, sizeof(self->macaddr));
    dev->priv = self;
    dev->ops = &usb_cdcether_net_ops;
    if (net_device_register(dev) == -1) {
        error("net_device_register() failure");
        memory_free(dev);
        return -1;
    }
    self->net_dev = dev;

    return 0;
}

void usb_cdcether_net_handler(void)
{
    struct net_device *dev = net_device_by_name("eth01");
    if (!dev) return;

    if (ether_input_helper(dev, usb_cdcether_receive_frame) == 0)
        intr_raise_irq(INTR_IRQ_SOFTIRQ);
}
