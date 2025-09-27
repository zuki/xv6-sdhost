//
// usb_request_.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014  R. Stange <rsta2@o2online.de>
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
#ifndef INC_USB_REQUEST_H
#define INC_USB_REQUEST_H

#include <usb/usb.h>
#include <usb/usb_endpoint.h>
#include <types.h>

struct usb_request;
typedef void usb_comp_cb(struct usb_request *urb, void *param, void *ctx);

typedef struct usb_request {
    struct usb_endpoint *ep;                 ///< エンドポイント
    usb_setup_data_t *  setup_data;         ///< セットアップデータ
    const void *        buffer;             ///< バッファ
    uint32_t            buflen;             ///< バッファ長
    int                 status;             ///< ステータス
    uint32_t            resultlen;          ///< 実際の長さ
    usb_comp_cb *       cb;                 ///< 完了時cbルーチン
    void *              param;              ///< cbルーチン引数
    void *              ctx;                ///< 完了時コンテキスト
    boolean             onnak;              ///< NAK受診時に完了とするか
    usb_error_t         error;              ///< error
} usb_request_t;

void usb_request(usb_request_t *self, struct usb_endpoint *ep, const void *buffer, uint32_t buflen, usb_setup_data_t *setup_data);
void _usb_request(usb_request_t *self);

struct usb_endpoint *usb_request_get_ep(usb_request_t *self);

void usb_request_set_status(usb_request_t *self, int status);
void usb_request_set_resultlen(usb_request_t *self, uint32_t resultlen);

int usb_request_get_status(usb_request_t *self);
uint32_t usb_request_get_resultlen(usb_request_t *self);

usb_setup_data_t *usb_request_get_setup_data(usb_request_t *self);
const void *usb_request_get_buffer(usb_request_t *self);
uint32_t usb_request_get_buflen(usb_request_t *self);

void usb_request_set_comp_cb(usb_request_t *self, usb_comp_cb *cb, void *param, void *ctx);
void usb_request_call_comp_cb(usb_request_t *self);

void usb_request_set_comp_on_nak(usb_request_t *self);
boolean usb_request_is_comp_on_nak(usb_request_t *self);

#endif
