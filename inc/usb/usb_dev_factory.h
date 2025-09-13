//
// usbdevicefactory.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2021  R. Stange <rsta2@o2online.de>
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
#ifndef INC_USB_DEV_FACTORY_H
#define INC_USB_DEV_FACTORY_H

#include <usb/usb_function.h>
#include <types.h>


#define USB_DEVICE(vendorid, deviceid)        vendorid, deviceid

/// @brief デバイスID構造体
typedef struct usb_device_factory
{
    uint16_t    vendor_id;
    uint16_t    device_id;
} usb_dev_factory_t;

/// @brief 指定のデバイス名またはインタフェース名のデバイスを取得する
/// @param parent 親デバイスオブジェクトへのポインタ
/// @param name インタフェース名(venxxx-xxx, intx-x-x)
/// @return デバイスへのポインタ。見つからなかった場合は0
usb_functon_t *usb_dev_factory_get_device(usb_functon_t *parent, char *name);

#endif
