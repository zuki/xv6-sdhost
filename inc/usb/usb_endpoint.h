//
// usbendpoint.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright(C) 2014-2019  R. Stange <rsta2@o2online.de>
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
#ifndef INC_USB_ENDPOINT_H
#define INC_USB_ENDPOINT_H

#include <types.h>
#include <usb/usb.h>
#include <usb/usb_device.h>

/// @brief エンドポイント転送種別
typedef enum {
    ep_type_control,
    ep_type_bulk,
    ep_type_interrupt,
    ep_type_isochronous
} usb_endpoint_type_t;

/// @brief USBエンドポイント構造体

typedef struct usb_endpoint {
    struct usb_device * dev;                ///< デバイス
    uint8_t             num;                ///< エンドポイント番号
    usb_endpoint_type_t type;               ///< 種別
    boolean             in;                 ///< 転送方向
    uint32_t            xsize;              ///< 最大パケットサイズ
    unsigned            interval;           ///< 間隔（ミリ秒）
    usb_pid_t           nextpid;            ///< 次のPID
} usb_endpoint_t;

void usb_endpoint_init(void);
usb_endpoint_t *usb_endpoint_alloc(struct usb_device *dev, const usb_ep_desc_t *desc);
void usb_endpoint_free(usb_endpoint_t *self);

// その他
usb_pid_t usb_endpoint_get_nextpid(usb_endpoint_t *self, boolean ststatus);
void usb_endpoint_skip_pid(usb_endpoint_t *self, unsigned packets, boolean ststatus);
void usb_endpoint_reset_pid(usb_endpoint_t *self);

void usb_endpoint_debug(usb_endpoint_t *self);

#endif
