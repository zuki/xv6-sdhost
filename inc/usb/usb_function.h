//
// usbfunction.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2019  R. Stange <rsta2@o2online.de>
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
#ifndef INC_USB_FUNCTION_H
#define INC_USB_FUNCTION_H

#include <usb/usb_cfg_parser.h>
#include <usb/usb.h>
#include <types.h>
#include <usb/usb.h>

struct usb_device;
struct dwhc_device;
struct usb_endpoint;

/// @brief デバイス機能クラスを表す構造体
typedef struct usb_function {
    boolean        (*configure)(struct usb_function *self);
    struct usb_device  *dev;           ///< デバイス
    usb_usb_cfg_parser_t    *usb_cfg_parser;    ///< コンフィグレーションパーサ
    usb_if_desc_t       *if_desc;       ///< インタフェースディスクリプタ
} usb_function_t;

void usb_function(usb_function_t *self, struct usb_device *dev, usb_usb_cfg_parser_t *parser);
void _usb_function(usb_function_t *self);
void usb_function_copy(usb_function_t *self, usb_function_t *func);

boolean usb_function_init(usb_function_t *self);
boolean usb_function_config(usb_function_t *self);
boolean usb_function_rescan_dev(usb_function_t *self);
boolean usb_function_remove_device(usb_function_t *self);
char *usb_function_get_if_name(usb_function_t *self);
uint8_t usb_function_get_num_eps(usb_function_t *self);
boolean usb_function_select_if(usb_function_t *self, uint8_t class, uint8_t subclass, uint8_t proto);

struct usb_device *usb_function_get_dev(usb_function_t *self);
struct usb_endpoint *usb_function_get_ep0(usb_function_t *self);
struct dwhc_device *usb_function_get_host(usb_function_t *self);

const usb_desc_t *usb_function_get_desc(usb_function_t *self, uint8_t type);

uint8_t usb_function_get_if_num(usb_function_t *self);
uint8_t usb_function_get_if_class(usb_function_t *self);
uint8_t usb_function_get_if_subclass(usb_function_t *self);
uint8_t usb_function_get_if_proto(usb_function_t *self);

#endif
