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
#include <usb.h>
#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_dev_ns.h>
#include <usb/usb_function.h>
#include <usb/usb_string.h>
#include <usb/lan7800.h>
#include <usb/usb_cdcether.h>
#include <usb/usb_keyboard.h>

static usb_lib_t *usb_lib = 0;

void usb_init(void)
{
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
    if (usb_lib->eth00 && lan7800_net_init(usb_lib->eth00) != 0)
        panic("failed to initialize lan7800");

    usb_lib->eth01 = (usb_cdcether_t *)usb_device_ns_get_dev(usb_device_ns_get(), "eth01", false);
    if (usb_lib->eth01 && usb_cdcether_net_init(usb_lib->eth01) != 0)
        panic("failed to initialize usb_cdcether");

    info("usb_init ok");
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

int usb_ethernet_available(void)
{
    assert(usb_lib != 0);
    return usb_lib->eth00 != 0 || usb_lib->eth01 != 0;
}

void usb_get_MACAddress(unsigned char buffer[6])
{
    assert(usb_lib != 0);
    assert(buffer != 0);

    if (usb_lib->eth00 != 0) {
        memmove(buffer, lan7800_get_macaddr(usb_lib->eth00), 6);
    } else if (usb_lib->eth01 != 0) {
        memmove(buffer, usb_cdcether_get_macaddr(usb_lib->eth01), 6);
    } else {
        error("no ethernet device available");
    }
}

int usb_ethernet_is_linkup(void)
{
    assert (usb_lib != 0);

    if (usb_lib->eth00 != 0)
    {
        return lan7800_is_linkup(usb_lib->eth00) ? 1 : 0;
    } else if (usb_lib->eth01 != 0) {
        return 1;
    }
    error("no ethernet device available");
    return 0;
}

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
