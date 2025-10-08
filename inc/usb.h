//
// INC_USB_H
//
// Services provided by the USPi library
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
#ifndef INC_USB_H
#define INC_USB_H

#include <types.h>
#include <usb/usb.h>
#include <usb/dwhc_device.h>
#include <usb/usb_dev_ns.h>
#include <usb/usb_hub.h>
#include <usb/lan7800.h>
#include <usb/usb_cdcether.h>
#include <usb/usb_keyboard.h>

#define MAX_DEVICES 4

typedef struct usb_library
{
    usb_device_ns_t      ns;
    dwhc_device_t        dwhc;
    usb_keyboard_t      *kbd;
    lan7800_t			*eth00;
	usb_cdcether_t		*eth01;
} usb_lib_t;

/// USBライブラリ

// usbライブラリを初期化する
// 失敗した場合はpanic
void usb_init(void);

// USBdescription用のメモリ割り当てと解放
usb_dev_desc_t *usb_dev_desc_alloc(void);
void usb_dev_desc_free(usb_dev_desc_t *self);

usb_cfg_desc_t *usb_cfg_desc_alloc(void);
void usb_cfg_desc_free(usb_cfg_desc_t *self);

usb_if_desc_t *usb_if_desc_alloc(void);
void usb_if_desc_free(usb_if_desc_t *self);

usb_ep_desc_t *usb_ep_desc_alloc(void);
void usb_ep_desc_free(usb_ep_desc_t *self);

usb_str_desc_t *usb_str_desc_alloc(void);
void usb_str_desc_free(usb_str_desc_t *self);

hub_desc_t *hub_desc_alloc(void);
void hub_desc_free(hub_desc_t *self);

void *usb_4byte_alloc(void);
void usb_4byte_free(void *self);


// keyboard
int usb_keyboard_available(void);
void usb_keyboard_register_key_pressed_handler(key_pressed_hdl *handler);
void usb_keyboard_register_shutdown_handler(shutdown_hdl *handler);
void usb_keyboard_register_key_status_handler(key_status_hdl *handler);
void usb_keyboard_update_leds(void);
void usb_keyboard_set_leds(uint8_t mask);

//
// USBデバイス情報
//

#define KEYBOARD_CLASS  1
#define MOUSE_CLASS     2
#define STORAGE_CLASS   3
#define ETHERNET_CLASS  4
#define GAMEPAD_CLASS   5
#define MIDI_CLASS      6

typedef struct usb_device_information
{
	// from USB device descriptor
	unsigned short	vendorid;
	unsigned short	productid;
	unsigned short	device;

	// points to a buffer in the USPi library, empty string if not available
	const char	*manufacturer;
	const char	*product;
} usb_device_info_t;

// 失敗の場合は0を返す
int usb_device_get_information(unsigned klass,        // クラスコード
                               unsigned index,  // 0-based index
                     usb_device_info_t *info); // provided buffer is filled

#endif
