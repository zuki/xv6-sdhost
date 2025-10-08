//
// usb.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#include <types.h>
#include <console.h>
#include <mm.h>
#include <slab.h>
#include <usb.h>
#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_hub.h>
#include <usb/usb_dev_ns.h>
#include <usb/usb_function.h>
#include <usb/usb_string.h>
#include <usb/lan7800.h>
#include <usb/usb_cdcether.h>
#include <usb/usb_keyboard.h>
#include <usb/usb_hub.h>

static usb_lib_t *usb_lib = 0;

static struct slab_cache *USB_DEV_DESC;
static struct slab_cache *USB_CFG_DESC;
static struct slab_cache *USB_IF_DESC;
static struct slab_cache *USB_EP_DESC;
static struct slab_cache *USB_STR_DESC;
static struct slab_cache *HUB_DESC;
static struct slab_cache *USB_4BYTE;

void usb_init(void)
{
    USB_DEV_DESC = slab_cache_create("usb_dev_desc", sizeof(usb_dev_desc_t), 64);
    USB_CFG_DESC = slab_cache_create("usb_cfg_desc", sizeof(usb_cfg_desc_t), 64);
    USB_IF_DESC = slab_cache_create("usb_if_desc", sizeof(usb_if_desc_t), 64);
    USB_EP_DESC = slab_cache_create("usb_ep_desc", sizeof(usb_dev_desc_t), 64);
    USB_STR_DESC = slab_cache_create("usb_str_desc", sizeof(usb_str_desc_t), 64);
    HUB_DESC = slab_cache_create("hub_desc", sizeof(hub_desc_t), 64);
    USB_4BYTE = slab_cache_create("usb_4byte", 4, 64);

    usb_lib = (usb_lib_t *)kmalloc(sizeof(usb_lib_t));
    assert(usb_lib != 0);
    usb_device_ns(&usb_lib->ns);
    dwhc_device(&usb_lib->dwhc);
    usb_lib->kbd = 0;
    usb_lib->eth00 = 0;
    usb_lib->eth01 = 0;

    if (dwhc_init(&usb_lib->dwhc, true)) {
        info("dwhc initialized\n");
    } else {
        _dwhc_device(&usb_lib->dwhc);
        _usb_device_ns(&usb_lib->ns);
        kmfree(usb_lib);
        panic("failed to initialize dw2_hc\n");
    }

    usb_lib->kbd = (usb_keyboard_t *)usb_device_ns_get_dev(usb_device_ns_get(), "kbd1", false);

    usb_lib->eth00 = (lan7800_t *)usb_device_ns_get_dev(usb_device_ns_get(), "eth00", false);
    trace("eth00: %p", usb_lib->eth00);
    if (usb_lib->eth00 && lan7800_net_init(usb_lib->eth00) != 0)
        panic("failed to initialize lan7800");

    usb_lib->eth01 = (usb_cdcether_t *)usb_device_ns_get_dev(usb_device_ns_get(), "eth01", false);
    trace("eth01: %p", usb_lib->eth01);
    if (usb_lib->eth01 && usb_cdcether_net_init(usb_lib->eth01) != 0)
        panic("failed to initialize usb_cdcether");

    info("usb_init ok");
}

usb_dev_desc_t *usb_dev_desc_alloc(void)
{
    return (usb_dev_desc_t *)slab_cache_alloc(USB_DEV_DESC);
}

void usb_dev_desc_free(usb_dev_desc_t *self)
{
    slab_cache_free(USB_DEV_DESC, self);
}

usb_cfg_desc_t *usb_cfg_desc_alloc(void)
{
    return (usb_cfg_desc_t *)slab_cache_alloc(USB_CFG_DESC);
}

void usb_cfg_desc_free(usb_cfg_desc_t *self)
{
    slab_cache_free(USB_CFG_DESC, self);
}

usb_if_desc_t *usb_if_desc_alloc(void)
{
    return (usb_if_desc_t *)slab_cache_alloc(USB_IF_DESC);
}

void usb_if_desc_free(usb_if_desc_t *self)
{
    slab_cache_free(USB_IF_DESC, self);
}

usb_ep_desc_t *usb_ep_desc_alloc(void)
{
    return (usb_ep_desc_t *)slab_cache_alloc(USB_EP_DESC);
}

void usb_ep_desc_free(usb_ep_desc_t *self)
{
    slab_cache_free(USB_EP_DESC, self);
}

usb_str_desc_t *usb_str_desc_alloc(void)
{
    return (usb_str_desc_t *)slab_cache_alloc(USB_STR_DESC);
}

void usb_str_desc_free(usb_str_desc_t *self)
{
    slab_cache_free(USB_STR_DESC, self);
}

hub_desc_t *hub_desc_alloc(void)
{
    return (hub_desc_t *)slab_cache_alloc(HUB_DESC);
}

void hub_desc_free(hub_desc_t *self)
{
    slab_cache_free(HUB_DESC, self);
}

void *usb_4byte_alloc(void)
{
    return slab_cache_alloc(USB_4BYTE);
}

void usb_4byte_free(void *self)
{
    slab_cache_free(USB_4BYTE, self);
}

int usb_keyboard_available(void)
{
    assert(usb_lib != 0);
    return usb_lib->kbd != 0;
}

void usb_keyboard_register_key_pressed_handler(key_pressed_hdl *handler)
{
    assert(usb_lib != 0);
    assert(usb_lib->kbd != 0);
    usb_keyboard_register_key_pressed_hdl(usb_lib->kbd, handler);
}

void usb_keyboard_register_shutdown_handler(shutdown_hdl *handler)
{
    assert(usb_lib != 0);
    assert(usb_lib->kbd != 0);
    usb_keyboard_register_shutdown_hdl(usb_lib->kbd, handler);
}

void usb_keyboard_register_key_status_handler(key_status_hdl *handler)
{
    assert(usb_lib != 0);
    assert(usb_lib->kbd != 0);
    usb_keyboard_register_key_status_hdl(usb_lib->kbd, handler);
}

void usb_keyboard_set_leds(uint8_t mask)
{
    assert(usb_lib != 0);
    assert(usb_lib->kbd != 0);
    usb_keyboard_set_led(usb_lib->kbd, mask);
}

void usb_keyboard_update_leds(void)
{
    assert(usb_lib != 0);
    assert(usb_lib->kbd != 0);
    usb_keyboard_update_led(usb_lib->kbd);
}

#if 0
int usb_send_frame (const void *buffer, unsigned length)
{
    boolean ret = false;
    assert (usb_lib != 0);

    if (usb_lib->eth00 != 0) {
        ret = lan7800_send_frame(usb_lib->eth00, buffer, length);
    } else if (usb_lib->eth01 != 0) {
        ret = usb_cdcether_send_frame(usb_lib->eth01, buffer, length);
    }

    if (!ret)
        warn("error has occured or no ethernet device available");
    return ret;
}

int usb_receive_frame (void *buffer, unsigned *resultlen)
{
    boolean ret = false;
    assert (usb_lib != 0);

    if (usb_lib->eth00 != 0) {
        ret = lan7800_receive_frame(usb_lib->eth00, buffer, resultlen);
    } else if (usb_lib->eth01 != 0) {
        ret = usb_cdcether_receive_frame(usb_lib->eth01, buffer, resultlen);
    }

    if (!ret)
        warn("error has occured or no ethernet device available");
    return ret;
}
#endif

int usb_device_get_information(unsigned klass, unsigned index, usb_device_info_t *info)
{
    assert(usb_lib != 0);

    usb_function_t *usb_function = 0;
    switch (klass) {
    case ETHERNET_CLASS:
        if (index == 0) {
            if (usb_lib->eth00 != 0) {
                usb_function = (usb_function_t *) usb_lib->eth00;
            } else if (usb_lib->eth01 != 0) {
                usb_function = (usb_function_t *) usb_lib->eth01;
            }
        }
        break;
    default:
        break;
    }

    if (usb_function == 0)
    {
        return 0;
    }

    usb_dev_t *usb_dev = usb_function_get_dev(usb_function);
    assert (usb_dev != 0);

    const usb_dev_desc_t *desc = usb_dev_get_dev_desc(usb_dev);
    assert (desc != 0);

    assert (info != 0);
    info->vendorid  = desc->vendorid;
    info->productid = desc->productid;
    info->device    = desc->device;

    info->manufacturer = usb_string_get(usb_dev->manufact);
    info->product      = usb_string_get(usb_dev->product);

    return 1;
}
