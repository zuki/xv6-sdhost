/*
 * WPA Supplicant - Layer2 packet interface definition
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file defines an interface for layer 2 (link layer) packet sending and
 * receiving. l2_packet_linux.c is one implementation for such a layer 2
 * implementation using Linux packet sockets and l2_packet_pcap.c another one
 * using libpcap and libdnet. When porting %wpa_supplicant to other operating
 * systems, a new l2_packet implementation may need to be added.
 */

#ifndef H_WLAN_L2_PACKET_H
#define H_WLAN_L2_PACKET_H

#include <utils/common.h>

/**
 * struct l2_packet_data - l2_packet内部データ構造体
 *
 * l2_packetの実装でそのプライベートデータを格納するために使用する。
 * 他のファイルはl2_packet関数を呼び出す際にこのデータへのポインタを使用する。
 * ただし、この構造体の内容をl2_packet実装の外から直接使用してはならない。
 */
struct l2_packet_data;

struct l2_ethhdr {
    uint8_t h_dest[ETH_ALEN];
    uint8_t h_source[ETH_ALEN];
    be16 h_proto;
} STRUCT_PACKED;


enum l2_packet_filter_type {
    L2_PACKET_FILTER_DHCP,
    L2_PACKET_FILTER_NDISC,
    L2_PACKET_FILTER_PKTTYPE,
};

/**
 * l2_packet_init - l2_packetインタフェースを初期化する
 * @ifname: インタフェース名
 * @own_addr: ドライバインタフェースから分かる場合にセットされるオプションの
 *    MACアドレス。わからない場合は %NULL
 * @protocol: ホストバイトオーダのEthernetプロトコル番号
 * @rx_callback: パケット受信時に呼び出されるコールバック関数
 * @rx_callback_ctx: rx_callback()を呼び出す際のコールバックデータ (ctx)
 * @l2_hdr: 1 = layer 2 ヘッダーを含む, 0 = ヘッダーを含まない
 * Returns: 内部データへのポインタ、失敗した場合は %NULL
 *
 * rx_callback 関数はsrc_addrにパケットの送信元アドレス（MACアドレス）をセットして
 * 呼び出される。l2_hdr が 0 の場合、bufはレイヤ 2 ヘッダー直後のlenバイト分の
 * ペイロードを指しており、同様に TX バッファもペイロードから始まる。この動作は
 * l2_hdr=1 とすることでデータバッファにレイヤ 2 ヘッダーを含めるように変更できる。
 *
 * rx_callback が NULL の場合、受信操作はまったく開始されない。すなわち、TX パスと
 * MACアドレスとIPアドレスを取得するための追加のヘルパー関数のみが使用可能となる。
 */
struct l2_packet_data * l2_packet_init(
    const char *ifname, const uint8_t *own_addr, unsigned short protocol,
    void (*rx_callback)(void *ctx, const uint8_t *src_addr, const uint8_t *buf, size_t len),
    void *rx_callback_ctx, int l2_hdr);

/**
 * l2_packet_init_bridge - l2_packet_init()と同様であるが、ブリッジ
 * 回避策ありで初期化する。
 *
 * このバージョンの l2_packet_init() を使用すると、ブリッジ内のステーション
 * インタフェースの場合に、Linuxパケットソケットの回避策を有効にすることができる。
 */
struct l2_packet_data * l2_packet_init_bridge(
    const char *br_ifname, const char *ifname, const uint8_t *own_addr,
    unsigned short protocol,
    void (*rx_callback)(void *ctx, const uint8_t *src_addr,
                const uint8_t *buf, size_t len),
    void *rx_callback_ctx, int l2_hdr);

/**
 * l2_packet_deinit - l2_packetインタフェースを破棄する
 * @l2: Pointer to internal l2_packet data from l2_packet_init()
 */
void l2_packet_deinit(struct l2_packet_data *l2);

/**
 * l2_packet_get_own_addr - 自身のレイヤ２アドレス（MACアドレス）を取得する
 * @l2: Pointer to internal l2_packet data from l2_packet_init()
 * @addr: 自身のMACアドレスを格納するバッファ（6バイト）
 * Returns: 0 on success, -1 on failure
 */
int l2_packet_get_own_addr(struct l2_packet_data *l2, uint8_t *addr);

/**
 * l2_packet_send - パケットを送信する
 * @l2: Pointer to internal l2_packet data from l2_packet_init()
 * @dst_addr: パケットの宛名アドレス (l2_hdr == 0 の場合にのみ使用する)
 * @proto: パケットのホストバイトオーダのプロトコル/ethertype
 *  (l2_hdr == 0 の場合にのみ使用する)
 * @buf: 送信するパケットの内容; l2_hdr=1の場合はレイヤ２ヘッダーを含む、
 *  l2_hdr=0の場合はペイロードだけが含まれる
 * @len: バッファ長 (l2_hdr == 1 の場合はヘッダーを含む)
 * Returns: >=0 on success, <0 on failure
 */
int l2_packet_send(struct l2_packet_data *l2, const uint8_t *dst_addr, uint16_t proto,
           const uint8_t *buf, size_t len);

/**
 * l2_packet_get_ip_addr - インタフェースの現在のIPアドレスを取得する
 * @l2: Pointer to internal l2_packet data from l2_packet_init()
 * @buf: テキスト形式のIPアドレスを格納するバッファ
 * @len: 最大バッファ長
 * Returns: 0 on success, -1 on failure
 *
 * この関数はl2_packetにバインドされているインタフェースから現在のIPアドレスを
 * 取得する。これは主にステータス情報を取得するためのものであり、IPアドレスは
 * ASCII文字列として格納される。この関数は%wpa_supplicantの動作に必須ではないため、
 * 完全な実装は求められない。
 * l2_packetの実装ではこの関数を定義する必要があるが、IPアドレス情報が利用できない
 * 場合は-1を返すことができる。
 */
int l2_packet_get_ip_addr(struct l2_packet_data *l2, char *buf, size_t len);


/**
 * l2_packet_notify_auth_start - 認証の開始をl2_packetに伝える
 * @l2: Pointer to internal l2_packet data from l2_packet_init()
 *
 * この関数は認証が開始されると予想される時点（たとえば、アソシエーションが完了した
 * ときなど）に呼び出され、EAPOLフレームに対するl2_packetの実装を準備するために
 * 使用される。この関数は主にl2_packetのコードでポーリングを行う必要がある場合に
 * 使用され、その場合はポーリングの頻度を高めることができる。また、l2_packetの
 * 実装において、認証の開始を把握してもメリットがない場合はこの関数を空の関数としても
 * 構わない。
 */
void l2_packet_notify_auth_start(struct l2_packet_data *l2);

/**
 * l2_packet_set_packet_filter - l2_packet用のソケットフィルタをセットする
 * @l2: Pointer to internal l2_packet data from l2_packet_init()
 * @type: enum l2_packet_filter_type, type of filter
 * Returns: 0 on success, -1 on failure
 *
 * この関数はl2_packetソケット用のソケットフィルタをセットするのに使用する.
 *
 */
int l2_packet_set_packet_filter(struct l2_packet_data *l2,
                enum l2_packet_filter_type type);

#endif /* L2_PACKET_H */
