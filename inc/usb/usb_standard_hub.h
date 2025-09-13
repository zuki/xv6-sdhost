//
// usbstandardhub.h
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
#ifndef INC_USB_STANDARD_HUB_H
#define INC_USB_STANDARD_HUB_H

#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_hub.h>
#include <usb/usb_function.h>
#include <usb/usb_request.h>
#include <types.h>
#include <string.h>

struct usb_endpoint;
struct usb_device;
struct usb_request;

typedef struct usb_standard_hub {
	usb_functon_t          func;       ///< usb functionオブジェクト
    struct usb_endpoint      *intr_ep;    ///< 割り込みエンドポイント
    hub_desc_t         *hub_desc;   ///< ハブディスクリプタ
    uint8_t            *buffer;     ///< ステータス変更データ用のバッファ
    unsigned            nports;     ///< ポート数
    boolean             poweron;    ///< 電源オンか
    uint32_t            devno;      ///< デバイス番号("uhubXX"のXX部)
    struct usb_device     *devs[USB_HUB_MAX_PORTS];    ///< ポートデバイス配列
    usb_port_status_t  *status[USB_HUB_MAX_PORTS];  ///< ポートステータス配列
    boolean             portconf[USB_HUB_MAX_PORTS]; ///< ポート構成済み配列
} usb_standard_hub_t;

void usb_standardhub(usb_standard_hub_t *self, usb_functon_t *func);
void _usb_standardhub(usb_standard_hub_t *self);
boolean usb_standard_hub_config(usb_functon_t *self);
boolean usb_standard_hub_rescan_dev(usb_standard_hub_t *self);
boolean usb_standard_hub_remove_dev(usb_standard_hub_t *self, uint32_t index);
boolean usb_standard_hub_disable_port(usb_standard_hub_t *self, uint32_t index);
void usb_standard_hub_handle_port_status_change(usb_standard_hub_t *self);

boolean usb_standard_hub_start_status_change_req(usb_standard_hub_t *self);
void usb_standard_hub_comp_cb(usb_standard_hub_t *self, struct usb_request *urb);
void usb_standard_hub_comp_cbstub(struct usb_request *urb, void *param, void *ctx);

void usb_standard_hub_port_status_changed(usb_standard_hub_t *self);

#endif
