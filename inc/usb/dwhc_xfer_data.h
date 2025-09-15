//
// dwhcixferstagedata.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright(C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#ifndef INC_USB_DWHC_XFER_DATA_H
#define INC_USB_DWHC_XFER_DATA_H

#include <usb/usb.h>
#include <usb/usb_request.h>
#include <usb/usb_device.h>
#include <usb/usb_endpoint.h>
#include <usb/dwhc_scheduler.h>
#include <usb/dwhc_periodic.h>
#include <usb/dwhc_non_periodic.h>
#include <usb/dwhc_non_split.h>
#include <types.h>

#define USB_TIMEOUT_NONE    0    // Wait forever

typedef struct dwhc_xfer_data {
    unsigned        channel;            ///< チャネル
    usb_request_t      *urb;                ///< リクエスト
    boolean         in;                 ///< 方向（INか）
    boolean         ststatus;           ///< ステージステータス

    boolean         split;              ///< 分割トランザクションか
    boolean         split_comp;         ///< 分割完了か
    unsigned        timeout;            ///< タイムアウト

    usb_dev_t      *dev;                ///< デバイス
    usb_endpoint_t       *ep;                 ///< エンドポイント
    usb_speed_t     speed;              ///< 速度
    uint32_t        xpsize;             ///< 最大パケットサイズ

    uint32_t        xfersize;           ///< 転送サイズ
    unsigned        packets;            ///< パケット数
    uint32_t        bpt;                ///< トランザクションあたりのバイト数
    unsigned        ppt;                ///< トランザクションあたりのパケット数
    uint32_t        xfered;             ///< 転送済み層バイト数

    unsigned        state;              ///< 状態
    unsigned        substate;           ///< 副状態
    uint32_t        trstatus;           ///< トランザクション状態
    unsigned        err_cnt;            ///< エラー数

    uint32_t        buffer[16] GALIGN(4);   ///< DMA buffer
    const void     *buffp;              ///< バッファへのポインタ

    unsigned        start;              ///< スタート時(tickHZ)

    boolean         fsused;
    union {
        dwhc_scheduler_t    base;
        dwhc_periodic_t     periodic;
        dwhc_non_periodic_t nonperiodic;
        dwhc_non_split_t    nosplit;
    } scheduler;                           ///< フレームスケジューラ
} dwhc_xfer_data_t;

void dwhc_xfer_data(dwhc_xfer_data_t *self, unsigned channel, usb_request_t *urb, boolean in, boolean ststatus, unsigned timeout);

void _dwhc_xfer_data(dwhc_xfer_data_t *self);

// change status
void dwhc_xfer_data_trans_complete(dwhc_xfer_data_t *self, uint32_t status, uint32_t packetleft, uint32_t byteleft);
void dwhc_xfer_data_set_split_complete(dwhc_xfer_data_t *self, boolean comp);

void dwhc_xfer_data_set_state(dwhc_xfer_data_t *self, unsigned state);
unsigned dwhc_xfer_data_get_state(dwhc_xfer_data_t *self);
void dwhc_xfer_data_set_sub_state(dwhc_xfer_data_t *self, unsigned substate);
unsigned dwhc_xfer_data_get_sub_state(dwhc_xfer_data_t *self);

boolean dwhc_xfer_data_begin_split_cycle(dwhc_xfer_data_t *self);

// get transaction parameters
unsigned dwhc_xfer_data_get_channel_number(dwhc_xfer_data_t *self);
uint8_t dwhc_xfer_data_get_dev_addr(dwhc_xfer_data_t *self);
boolean dwhc_xfer_data_is_periodic(dwhc_xfer_data_t *self);
uint8_t dwhc_xfer_data_get_ep_type(dwhc_xfer_data_t *self);
uint8_t dwhc_xfer_data_get_ep_number(dwhc_xfer_data_t *self);
uint32_t dwhc_xfer_data_get_max_packet_size(dwhc_xfer_data_t *self);
usb_speed_t dwhc_xfer_data_get_speed(dwhc_xfer_data_t *self);

uint8_t dwhc_xfer_data_get_pid(dwhc_xfer_data_t *self);
boolean dwhc_xfer_data_is_in(dwhc_xfer_data_t *self);
boolean dwhc_xfer_data_is_ststage(dwhc_xfer_data_t *self);

uint64_t dwhc_xfer_data_get_dmaaddr(dwhc_xfer_data_t *self);
uint32_t dwhc_xfer_data_get_byte2xfer(dwhc_xfer_data_t *self);
uint32_t dwhc_xfer_data_get_packet2xfer(dwhc_xfer_data_t *self);

boolean dwhc_xfer_data_is_split(dwhc_xfer_data_t *self);
boolean dwhc_xfer_data_is_split_complete(dwhc_xfer_data_t *self);
uint8_t dwhc_xfer_data_get_hubaddr(dwhc_xfer_data_t *self);
uint8_t dwhc_xfer_data_get_hubport(dwhc_xfer_data_t *self);
uint8_t dwhc_xfer_data_get_split_pos(dwhc_xfer_data_t *self);

uint32_t dwhc_xfer_data_get_status_mask(dwhc_xfer_data_t *self);

// check status after transaction
uint32_t dwhc_xfer_data_get_trstatus(dwhc_xfer_data_t *self);
boolean dwhc_xfer_data_is_stage_complete(dwhc_xfer_data_t *self);
uint32_t dwhc_xfer_data_get_resultlen(dwhc_xfer_data_t *self);

usb_request_t *dwhc_xfer_data_get_urb(dwhc_xfer_data_t *self);
dwhc_scheduler_t *dwhc_xfer_data_get_scheduler(dwhc_xfer_data_t *self);

boolean dwhc_xfer_data_is_retry_ok(dwhc_xfer_data_t *self);

usb_error_t dw2_xter_dtagedata_get_usb_err(dwhc_xfer_data_t *self);

boolean dwhc_xfer_data_is_timeout(dwhc_xfer_data_t *self);

void debug_stdata(dwhc_xfer_data_t *self);

#endif
