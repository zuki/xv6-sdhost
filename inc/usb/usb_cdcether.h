//
// usbusb_cdcethernet.h
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
#ifndef INC_USB_usb_cdcether_H
#define INC_USB_usb_cdcether_H

#include <types.h>
#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_endpoint.h>
#include <usb/usb_function.h>
#include <netdevice.h>

/// @brief CDC Ethernetデバイス構造体
typedef struct usb_usb_cdcethernet {
    usb_function_t   usb_func;
    net_dev_t   *net_dev;
    usb_endpoint_t    *bulk_in;       ///< バルク転送入力用パイプ
    usb_endpoint_t    *bulk_out;      ///< バルク転送出力用パイプ
    char         macaddr[MAC_ADDRESS_SIZE]; /// MACアドレス
} usb_cdcether_t;

void usb_cdcether(usb_cdcether_t *self, usb_function_t *func);
void _usb_cdcether(usb_cdcether_t *self);

boolean usb_cdcether_configure(usb_function_t *func);
boolean usb_cdcether_send_frame(usb_cdcether_t *self, const void *buffer, uint32_t len);
boolean usb_cdcether_receive_frame(usb_cdcether_t *self, void *buff, uint32_t *resultlen);
boolean usb_cdcether_init_macaddr(usb_cdcether_t *self, uint8_t id);

const char *usb_cdcether_get_macaddr(usb_cdcether_t *self);

#endif
