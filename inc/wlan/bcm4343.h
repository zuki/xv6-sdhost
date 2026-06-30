//
// bcm4343.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2020-2025  R. Stange <rsta2@gmx.net>
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
#ifndef INC_WLAN_BCM4343_H
#define INC_WLAN_BCM4343_H

#include <types.h>
#include <net/net.h>
#include <net/util.h>
#include <wlan/etherevent.h>
#include <spinlock.h>

typedef ether_event_handler_t bcm4343_event_hander;
typedef boolean bcm4343_connect_provider(void);

struct bcm4343 {
    struct net_device   *net_dev;   // ネットデバイスオブジェクト
    char *  firm_path;              // ファームウェアのパス
    uint8_t macaddr[6];             // MACアドレス
    uint8_t bssid[6];               // APのSSID

    struct spinlock lock;           // 以下の2つのqueueを保護
    struct queue_head rx_queue;     // 受信キュー
    struct queue_head scan_queue;   // 結果スキャンキュー

    boolean opennet;                // ネットワークはオープン済みか?
    boolean linkup;                 // ネットワークはLinkUp済みか?
    bcm4343_connect_provider *is_connected;    // 接続済みか確認する関数へのポインタ
};

int bcm4343_init(const char *firm_path);        // e.g. "/d/firmware/"
int bcm4343_deinit(struct bcm4343 *self);

static inline uint16_t bcm4343_gettype(void)
{
    return NET_DEVICE_TYPE_WLAN;
}

uint8_t *bcm4343_get_macaddr(struct bcm4343 *self);

boolean bcm4343_send_frame(struct bcm4343 *self, const void *buff, unsigned len);

// pBuffer must have size FRAME_BUFFER_SIZE
boolean bcm4343_recv_frame(struct bcm4343 *self, void *buff, unsigned *rlen);

boolean bcm4343_is_linkup(struct bcm4343 *self);

boolean bcm4343_set_mcast_filter(struct bcm4343 *self, const uint8_t Groups[][MAC_ADDRESS_SIZE]);

/// \param handler Pointer to event handler (0 for unregister)
/// \param context Pointer to be handed over to the handler
void bcm4343_register_event_handler(struct bcm4343 *self, bcm4343_event_hander *handler, void *context);

/// \param handler Pointer to connected provider)
void bcm4343_register_connect_provider(struct bcm4343 *self, bcm4343_connect_provider *handler);

/// \param format Device specific control command (0-terminated)
/// \return Operation successful?
boolean bcm4343_control(struct bcm4343 *self, const char *format, ...);

/// \brief Poll for a received scan result message
/// \param buff Message will be placed here, buffer must have size FRAME_BUFFER_SIZE
/// \param rlen Pointer to variable, which receives the valid message length
/// \return TRUE if a message is returned in buffer, FALSE if nothing has been received
boolean bcm4343_recv_scan_result(struct bcm4343 *self, void *buff, unsigned *rlen);

uint8_t *bcm4343_get_bssid(struct bcm4343 *self);

/// \param ssid SSID of open network to be joined
/// \return Operation successful?
boolean bcm4343_join_opennet(struct bcm4343 *self, const char *ssid);

/// \param ssid SSID of open network to be created
/// \param channel 802.11 channel of open network to be created
/// \param hidden Whether to hide the SSID
/// \return Operation successful?
boolean bcm4343_create_opennet(struct bcm4343 *self, const char *ssid, int channel, boolean hidden);

/// \brief Destroy created open network
boolean bcm4343_destroy_opennet(struct bcm4343 *self);

void bcm4343_dump_status(struct bcm4343 *self);

#endif

