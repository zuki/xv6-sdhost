//
// bcm4343.cpp
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
#include <types.h>
#include <wlan/bcm4343.h>
#include <wlan/p9compat.h>
#include <console.h>
#include <net/net.h>
#include <net/util.h>
#include <net/platform.h>
#include <net/ether.h>

extern void ether4330link(void);

struct bcm4343_queue_entry {
    unsigned len;
    uint8_t data[FRAME_BUFFER_SIZE];
};

static ether_pnp_t *ether_pnp_handler = NULL;
static Ether ether_device;

static struct bcm4343 *bcm4343 = 0;

static int bcm4343_init_internal(struct bcm4343 *self);
static void bcm4343_frame_received(struct bcm4343 *self, const void *buff, unsigned len);
static void bcm4343_scan_result_recv(struct bcm4343 *self, const void *buff, unsigned len);
static void bcm4343_opennet_event_handler(ether_event_type_t type,
        const ether_event_params_t *param, void *context);

static int bcm4343_open(struct net_device *dev) { return 0; }
static int bcm4343_close(struct net_device *dev) { return 0; }

static int bcm4343_linkup(struct net_device *dev)
{
    struct bcm4343 *self = (struct bcm4343 *)(dev->priv);
    // LinkUpされていればtrue
    if (self->opennet) {
        return self->linkup;
    }
    // is_connected()を実行して接続済であるか確認する
    if (self->is_connected != 0) {
        return (*self->is_connected)();
    }

    return false;
}

static int bcm4343_transmit(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst)
{
    return ether_transmit_helper(dev, type, data, len, dst, bcm4343_send_frame);
}

static struct net_device_ops bcm4343_ops = {
    .open = bcm4343_open,
    .close = bcm4343_close,
    .transmit = bcm4343_transmit,
    .linkup = bcm4343_linkup,
};

// sample/hello_wlan/kernel.cpp
//   firm_path : "SD:/firmware/" -> /d/firmware とする
int bcm4343_init(const char *firm_path)
{
    if (!bcm4343)
        bcm4343 = memory_alloc(sizeof(struct bcm4343));
    assert(bcm4343);

    struct net_device *net_dev;
    net_dev = net_device_alloc();
    assert(net_dev);
    net_dev->ops = &bcm4343_ops;
    net_dev->index = NET_INDEX_BCM4343;
    net_dev->type = NET_DEVICE_TYPE_WLAN;
    snprintf(net_dev->name, sizeof(net_dev->name), "net%d", net_dev->index);
    net_dev->priv = bcm4343;

    bcm4343->net_dev = net_dev;
    bcm4343->firm_path = firm_path;
    queue_init(&bcm4343->rx_queue);
    queue_init(&bcm4343->scan_queue);
    initlock(&bcm4343->lock, "bcm4343");
    bcm4343->opennet = false;
    bcm4343->linkup = false;
    bcm4343->is_connected = NULL;
    bcm4343->data = NULL;
    trace("bcm4343_init ok");
    return bcm4343_init_internal(bcm4343);
}

int bcm4343_deinit(struct bcm4343 *self)
{
    if (self->opennet)
        net_device_close(self->net_dev);

    if (self->net_dev)
        memory_free(self->net_dev);

    self = NULL;

    trace("deinit bcm4343");

    return 0;
}

static int bcm4343_init_internal(struct bcm4343 *self)
{
    // 1. p9アーキテクチャの初期化（DMAチャンネルのセットと周期timerハンドラの登録）
    p9arch_init();
    // 2. ファームウェアが存在するパスをfirm_pathにセット
    p9chan_init(self->firm_path);
    // 3. 現在のタスクにerror_stackをuserDataとしてセット
    p9proc_init();
    // 4. ether4343をethernetカードとして登録
    ether4330link();
    assert(ether_pnp_handler != NULL);    // addethercard()で設定
    // 5. ether4343ドライバの作成）
    ether_pnp_handler(&ether_device);
    ether_device.oq = kmzalloc(sizeof(Queue));
    //memset(ether_device.oq, 0, sizeof *ether_device.oq);

    if (waserror ()) {
        return -1;
    }
    /* ethernetデバイスをアタッチ */
    assert(ether_device.attach != 0);
    ether_device.attach(&ether_device);
    /* MACアドレスをセット */
    memmove(self->macaddr, ether_device.ea, MAC_ADDRESS_SIZE);
    /* 自分自身をnet_devのプライベートデータとして登録 */
    self->net_dev->priv = self;

    /* netデバイスとして登録 */
    if (net_device_register(self->net_dev) == -1) {
        p9error("failed net_device_register");
    }
    poperror();
    trace("bcm4343_init_internal ok");
    return 0;
}

uint8_t *bcm4343_get_macaddr(struct bcm4343 *self)
{
    return self->macaddr;
}

// バッファの内容を送信する
long bcm4343_send_frame(struct net_device *dev, const uint8_t *buff, uint64_t len)
{
    //struct bcm4343 *self = (struct bcm4343 *)dev;

    //hexdump (buff, len, "wlantx");

    // lenのサイズのブロックを割り当てる
    Block *block = allocb (len);
    assert(block != 0);

    assert(block->wp != 0);
    assert(buff != 0);
    // ブロックのwpからpBufferの内容をコピーする
    memcpy(block->wp, buff, len);
    block->wp += len;

    assert(ether_device.oq != 0);
    // ブロックをキューに登録する
    qpass(ether_device.oq, block);

    if (waserror ()) {
        return -1;
    }

    assert(ether_device.transmit != 0);
    // 転送を行う
    ether_device.transmit(&ether_device);

    poperror ();

    return 0;
}

// バッファにデータを読み込む(buffはetherヘッダー付きEthernetパケット)
long bcm4343_recv_frame(struct net_device *dev, uint8_t *buff, uint64_t *rlen)
{
    struct bcm4343 *self = (struct bcm4343 *)dev->priv;

    assert(buff != 0);
    struct bcm4343_queue_entry *entry;

    // 受信キューの先頭エントリをbuffに読み込む
    acquire(&self->lock);
    entry = (struct bcm4343_queue_entry *)queue_pop(&self->rx_queue);
    release(&self->lock);
    if (entry == NULL || entry->len == 0)
        return -1;
    assert(rlen != 0);
    memmove(buff, (const void *)entry->data, entry->len);
    *rlen = entry->len;
    kmfree(entry);
    //hexdump (buff, len, "wlanrx");

    return 0;
}

boolean bcm4343_set_mcast_filter(struct bcm4343 *self, const uint8_t Groups[][MAC_ADDRESS_SIZE])
{
    uint32_t nGroups = 0;
    while (Groups[nGroups][0]) {
        nGroups++;
    }

    size_t ulSize = sizeof nGroups + nGroups * MAC_ADDRESS_SIZE;

    uint8_t buffer[ulSize];
    memcpy(buffer, &nGroups, sizeof nGroups);
    if (nGroups) {
        memcpy(buffer + sizeof nGroups, Groups, nGroups * MAC_ADDRESS_SIZE);
    }

    if (waserror ()) {
        return false;
    }

    assert(ether_device.setmulticast != 0);
    // Groupsに含まれるアドレスをマルチキャストリストに登録する
    (*ether_device.setmulticast)(&ether_device, buffer, ulSize);

    poperror ();

    return true;
}

// イベントハンドラを登録する
void bcm4343_register_event_handler(struct bcm4343 *self, bcm4343_event_hander *handler, void *context)
{
    assert(ether_device.setevhndlr != 0);
    (*ether_device.setevhndlr)(&ether_device, handler, context);
}

// 接続確認用ハンドラを登録する
void bcm4343_register_connect_provider(struct bcm4343 *self, bcm4343_connect_provider *handler)
{
    self->is_connected = handler;
}

/* 制御コマンドを実行する */
boolean bcm4343_control(struct bcm4343 *self, const char *format, ...)
{
    assert(format != 0);

    char command[256];
    va_list ap;
    va_start(ap, format);
    vsnprintfmt(command, 256, format, ap);
    va_end(ap);

    if (waserror ()) {
        return false;
    }
    trace("command: '%s'", command);
    assert(ether_device.ctl != 0);
    // etherbcmctl()を実行
    (*ether_device.ctl)(&ether_device, (const char *)command, 0);

    poperror ();

    return true;
}

// スキャン結果を受信する
boolean bcm4343_recv_scan_result(struct bcm4343 *self, void *buff, unsigned *rlen)
{
    assert(buff != 0);
    struct bcm4343_queue_entry *entry;
    acquire(&self->lock);
    trace("bcm4343: %p, &scan_queue: %p", self, &self->scan_queue);
    entry = (struct bcm4343_queue_entry *)queue_pop(&self->scan_queue);
    trace("dequeue: entry: %p, len=0x%x, q->size: %d, isnull : %s", entry, entry->len, self->scan_queue.num, entry == NULL ? "yes" : "no");
    //hexdump (entry->data, entry->len, "entry");
    // FIXME: entry == 0 とならない件を解決する
    if (!entry || entry->len == 0) {
        debug("no more result");
        release(&self->lock);
        return false;
    }
    if (self->scan_queue.num == 0) {
        debug("queue num 0");
        release(&self->lock);
        return false;
    }
    release(&self->lock);
    assert(rlen != 0);
    memmove(buff, (const void *)entry->data, entry->len);
    *rlen = entry->len;
    kmfree(entry);
    //hexdump (buff, *rlen, "wlanscan");
    return true;
}

// ネットワークSSIDを取得する
uint8_t *bcm4343_get_bssid(struct bcm4343 *self)
{
    uint8_t BSSID[MAC_ADDRESS_SIZE];
    assert(ether_device.getbssid != 0);
    (*ether_device.getbssid)(&ether_device, BSSID);

    memmove(self->bssid, BSSID, MAC_ADDRESS_SIZE);

    return self->bssid;
}

// ネットワーク ssid に接続する
boolean bcm4343_join_opennet(struct bcm4343 *self, const char *ssid)
{
    self->opennet = self->linkup = false;

    bcm4343_register_event_handler(self, bcm4343_opennet_event_handler, self);

    assert(ssid != 0);
    boolean ok = bcm4343_control(self, "join %s %s 0 off", ssid, "FFFFFFFFFFFF");

    self->opennet = ok;

    return ok;
}

// by @sebastienNEC: ネットワークAPを作成する
boolean bcm4343_create_opennet(struct bcm4343 *self, const char *ssid, int channel, boolean hidden)
{
    self->opennet = self->linkup = false;

    assert(ssid != 0);
    boolean ok = bcm4343_control(self, "create %s %d %d", ssid, channel, hidden);

    self->opennet = self->linkup = ok;

    return ok;
}

// ネットワークAPを廃棄する
boolean bcm4343_destroy_opennet(struct bcm4343 *self)
{
    self->opennet = self->linkup = false;

    return bcm4343_control(self, "down");
}

// ifstatを出力する
void bcm4343_dump_status(void)
{
    char buffer[200];

    assert(ether_device.ifstat != 0);
    long len = (*ether_device.ifstat)(&ether_device, buffer, sizeof buffer, 0);
    buffer[len] = '\0';

    print(buffer);
}

// データフレームをbuffに受信する
static void bcm4343_frame_received(struct bcm4343 *self, const void *buff, unsigned len)
{
    assert(self != 0);
    struct bcm4343_queue_entry *entry = (struct bcm4343_queue_entry *)kmalloc(sizeof(*entry));
    memmove(entry->data, buff, len);
    entry->len = len;
    acquire(&self->lock);
    // 受信キューにbuffを登録する
    if (!queue_push(&self->rx_queue, entry)) {
        error("queue_push rx_queue failed");
        kmfree(entry);
        release(&self->lock);
        return;
    }
    // TODO: どうするか検討
    //intr_raise_irq(INTR_IRQ_SOFTIRQ);
    release(&self->lock);
}

// スキャン結果bufをキューに登録する
static void bcm4343_scan_result_recv(struct bcm4343 *self, const void *buff, unsigned len)
{
    assert(self != 0);
    struct bcm4343_queue_entry *entry = (struct bcm4343_queue_entry *)kmalloc(sizeof(*entry));
    memmove(entry->data, buff, len);
    entry->len = len;
    acquire(&self->lock);
    trace("bcm4343: %p, &scan_queue: %p", self, &self->scan_queue);
    trace("enqueue: entry: %p, len=0x%x, q->size: %d", entry, entry->len, self->scan_queue.num);
    // スキャン結果キューにbuffを登録する
    if (!queue_push(&self->scan_queue, entry)) {
        error("queue_push scan_queue failed");
        kmfree(entry);
        release(&self->lock);
        return;
    }
    // TODO: どうするか検討
    //intr_raise_irq(INTR_IRQ_SOFTIRQ);
    release(&self->lock);
}

// OpenNetイベントハンドラ
static void bcm4343_opennet_event_handler(ether_event_type_t type,
    const ether_event_params_t *param, void *context)
{
    struct bcm4343 *self = (struct bcm4343 *)context;
    assert(self != 0);

    switch (type) {
    case ether_event_link:
        self->linkup = true;
        break;

    case ether_event_disassoc:
        self->linkup = false;
        break;

    default:
        break;
    }
}

// Ethernetデータを受信する : b->rpはethernetヘッダーを指している
void etheriq(Ether *ether, Block *block, unsigned flag)
{
    // FIXME: flag = 1 で何をするべきか?
    assert(block != 0);
    bcm4343_frame_received(bcm4343, block->rp, BLEN(block));

    freeb(block);
}

// スキャン結果を受信する
void etherscanresult(Ether *ether, const void *buff, long len)
{
    assert(buff != 0);
    bcm4343_scan_result_recv(bcm4343, buff, (unsigned) len);
}

// pnpハンドラを登録してカードを追加する
void addethercard (const char *name, ether_pnp_t *handler)
{
    assert(handler != 0);
    ether_pnp_handler = handler;
}
