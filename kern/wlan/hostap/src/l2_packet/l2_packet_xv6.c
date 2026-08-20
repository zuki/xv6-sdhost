//
// l2_packet_circle.cpp
//
// Layer2 packet handling interface for Circle
// by R. Stange <rsta2@gmx.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//

#include <utils/includes.h>
#include <utils/common.h>
#include <utils/eloop.h>
#include <l2_packet/l2_packet.h>

#include <types.h>
#include <net/net.h>
#include <net/ether.h>
#include <proc.h>
#include <string.h>
#include <mm.h>
#include <net/ether.h>
#include <wlan/bcm4343.h>

// wifi経由のlayer 2(link層)の送受信を行う(BCM434の送受信関数を使用する)

#define SOCK_FD        1

// l2_packet内部データ構造体
struct l2_packet_data {
    unsigned short protocol;
    void (*rx_callback)(void *ctx, const uint8_t *src_addr, const uint8_t *buf, size_t len);
    void *rx_callback_ctx;
    uint8_t own_addr[ETHER_ADDR_LEN];
    struct net_device *dev;
};

static void l2_packet_receive(int sock, void *eloop_ctx, void *sock_ctx);

static int auth_active;

struct l2_packet_data * l2_packet_init(const char *ifname, const uint8_t *own_addr,
        unsigned short protocol,
        void (*rx_callback) (void *ctx, const uint8_t *src_addr,
                              const uint8_t *buf, size_t len),
        void *rx_callback_ctx, int l2_hdr)
{
    assert (own_addr == 0);         // この関数でセット
    assert (protocol == 0x888E);    // EAP-over-LAN
    assert (l2_hdr == 0);

    struct l2_packet_data *l2 = (struct l2_packet_data *) kmzalloc(sizeof *l2);
    if (l2 == 0) {
        return 0;
    }

    l2->protocol = protocol;
    l2->rx_callback = rx_callback;
    l2->rx_callback_ctx = rx_callback_ctx;

    auth_active = 0;

    struct net_device *net_dev = net_device_by_index(NET_INDEX_BCM4343);
    assert(net_dev);
    uint8_t *macaddr = ((struct bcm4343 *)net_dev->priv)->macaddr;
    memmove(l2->own_addr, macaddr, ETHER_ADDR_LEN);

    l2->dev = net_dev;
    assert (l2->dev != 0);
    l2->dev->type = protocol;

    if (eloop_register_read_sock(SOCK_FD, l2_packet_receive, l2, 0) < 0)
        return NULL;

    return l2;
}

struct l2_packet_data * l2_packet_init_bridge (const char *br_ifname, const char *ifname,
                    const uint8_t *own_addr, unsigned short protocol,
                    void (*rx_callback) (void *ctx, const uint8_t *src_addr,
                                 const uint8_t *buf, size_t len),
                    void *rx_callback_ctx, int l2_hdr)
{
    return l2_packet_init (br_ifname, own_addr, protocol, rx_callback, rx_callback_ctx, l2_hdr);
}

void l2_packet_deinit (struct l2_packet_data *l2)
{
    if (l2 == 0) {
        return;
    }

    eloop_unregister_read_sock(SOCK_FD);
    kmfree(l2);
}

int l2_packet_get_own_addr (struct l2_packet_data *l2, uint8_t *addr)
{
    assert (l2 != 0);
    assert (addr != 0);
    memcpy(addr, l2->own_addr, ETHER_ADDR_LEN);

    return 0;
}

int l2_packet_send (struct l2_packet_data *l2, const uint8_t *dst_addr,
    uint16_t proto, const uint8_t *buf, size_t len)
{
    if (l2 == 0) {
        return -1;
    }

    uint8_t buffer[FRAME_BUFFER_SIZE];
    struct ether_hdr *header = (struct ether_hdr *) buffer;
    assert (dst_addr != 0);
    memcpy(header->dst, dst_addr, MAC_ADDRESS_SIZE);
    memcpy(header->src, l2->own_addr, MAC_ADDRESS_SIZE);
    header->type = hton16(l2->protocol);

    assert (len > 0);
    if (len + sizeof (struct ether_hdr) > FRAME_BUFFER_SIZE) {
        return -1;
    }

    assert (buf != 0);
    memcpy(buffer + sizeof (struct ether_hdr), buf, len);

    assert (l2->dev != 0);
    bcm4343_send_frame(l2->dev, buffer, len + sizeof (struct ether_hdr));

    return 0;
}

static void l2_packet_receive (int sock, void *eloop_ctx, void *sock_ctx)
{
    assert (sock == SOCK_FD);
    struct l2_packet_data *l2 = (struct l2_packet_data *) eloop_ctx;
    assert (l2 != 0);
    assert (l2->dev != 0);
    assert (l2->rx_callback != 0);

    uint8_t buffer[FRAME_BUFFER_SIZE];
    uint64_t rlength;

    while (bcm4343_recv_frame(l2->dev, buffer, &rlength) == 0) {
        // bufferにはethernetヘッダーがあるのでsrc_addrを取り出した後、ヘッダーを削除する
        if (rlength <= sizeof(struct ether_hdr))
            continue;
        struct ether_hdr *header = (struct ether_hdr *)buffer;
        uint8_t src_addr[MAC_ADDRESS_SIZE];
        memcpy(src_addr, header->src, ETHER_ADDR_LEN);
        l2->rx_callback(l2->rx_callback_ctx, src_addr, buffer+sizeof(*header), rlength-sizeof(*header));

        yield ();
    }
}

#define AUTH_DURATION_SECS    5

static void l2_packet_auth_end (void *eloop_ctx, void *timeout_ctx)
{
    auth_active = 0;
}

void l2_packet_notify_auth_start (struct l2_packet_data *l2)
{
    assert (l2 != 0);

    auth_active = 1;

    eloop_cancel_timeout (l2_packet_auth_end, l2, 0);
    eloop_register_timeout (AUTH_DURATION_SECS, 0, l2_packet_auth_end, l2, 0);
}

int l2_packet_get_ip_addr (struct l2_packet_data *l2, char *buf, size_t len)
{
    return -1;
}

int l2_packet_set_packet_filter (struct l2_packet_data *l2, enum l2_packet_filter_type type)
{
    return -1;
}

int l2_packet_auth_active (void)
{
    return auth_active;
}
