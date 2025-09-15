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
#include <usb/dwhc_device.h>
#include <usb/usb_dev_ns.h>
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

// keyboard
int usb_keyboard_available(void);

void usb_keyboard_register_key_pressed_handler(key_pressed_hdl *handler);

void usb_keyboard_register_shutdown_handler(shutdown_hdl *handler);

void usb_keyboard_register_key_status_handler(key_status_hdl *handler);

void usb_keyboard_update_leds(void);

void usb_keyboard_set_leds(uint8_t mask);

// Ethernetデバイス
// (Ethernetにアクセスするのは、usb_init()の呼び出し後に2秒待ってから）

/* Ethernetデバイスが利用可能化
 * コントローラを確認するだけでイーサネットリンクが確立されているか
 * 否かは確認しない
 * 利用可能な場合は0以外を返す
 */
int usb_ethernet_available(void);

/* MACAddressを取得する */
void usb_get_MACAddress(unsigned char Buffer[6]);

/* Ethernetはリンクアップしているか.
 * している場合は0以外を返す
 */
int usb_ethernet_is_linkup(void);

/* フレームを送信する.
 * 失敗した場合は0を返す
 */
int usb_send_frame(const void *buffer, unsigned length);

/* フレームを受信する.
 * bufferのサイズはUSPI_FRAME_BUFFER_SIZEでなければならない
 * フレームが利用不可または失敗の場合は0を返す
 */
#define USPI_FRAME_BUFFER_SIZE	1600
int usb_receive_frame(void *buffer, unsigned *resultlen);

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
