# xv6-netのnetサブシステムを調査

## TCP/IPを実現しているコード

- 基幹: net/net.c, net/util.c
- 物理層: net/platform/xv6-riscv/virtio_net.c
- リンク層: net/ether.c, net/arp.c
- インターネット層: net/ip.c, net/icmp.c
- トランスポート層: net/udp.c, net/tcp.c
- ライブラリ: net/socket.c, syssocket.c
- アプリケーション層: user/echo.c, user/ifconfig.c, user/udpecho.c, user/tcpecho.c

## 構造体

### 基幹

```c
struct net_device {
    struct net_device *next;
    struct net_iface *ifaces;
    unsigned int index;
    char name[IFNAMSIZ];
    uint16_t type;
    uint16_t mtu;
    uint16_t flags;
    uint16_t hlen; /* header length */
    uint16_t alen; /* address length */
    uint8_t addr[NET_DEVICE_ADDR_LEN];
    union {
        uint8_t peer[NET_DEVICE_ADDR_LEN];
        uint8_t broadcast[NET_DEVICE_ADDR_LEN];
    };
    struct net_device_ops *ops;
    void *priv;
};

struct net_device_ops {
    int (*open)(struct net_device *dev);
    int (*close)(struct net_device *dev);
    int (*transmit)(struct net_device *dev, uint16_t type, const uint8_t *data, size_t size, const void *dst);
};

struct net_iface {
    struct net_iface *next;
    struct net_device *dev; /* back pointer to parent */
    int family;
    /* depends on implementation of protocols. */
};
```

### リンク層

```c
struct net_protocol {
    struct net_protocol *next;
    uint16_t type;
    struct queue_head queue; /* input queue */
    void (*handler)(const uint8_t *data, size_t len, struct net_device *dev);
};

struct net_protocol_queue_entry {
    struct net_device *dev;
    size_t len;
    uint8_t data[];
};

struct net_timer {
    struct net_timer *next;
    struct timeval interval;
    struct timeval last;
    void (*handler)(void);
};

struct net_event {
    struct net_event *next;
    void (*handler)(void *arg);
    void *arg;
};
```

### ネットワーク層

```c
struct ip_protocol {
    struct ip_protocol *next;
    uint8_t type;
    void (*handler)(const uint8_t *data, size_t len, ip_addr_t src, ip_addr_t dst, struct ip_iface *iface);
};

struct ip_endpoint {
    ip_addr_t addr;
    uint16_t port;
};

struct ip_iface {
    struct net_iface iface;
    struct ip_iface *next;
    ip_addr_t unicast;
    ip_addr_t netmask;
    ip_addr_t broadcast;
};

struct ip_route {
    struct ip_route *next;
    ip_addr_t network;
    ip_addr_t netmask;
    ip_addr_t nexthop;
    struct ip_iface *iface;
};
```

### トランスポート層

```c
struct tcp_pcb {
    int state;
    int mode; /* user command mode */
    struct ip_endpoint local;
    struct ip_endpoint foreign;
    struct {
        uint32_t nxt;
        uint32_t una;
        uint16_t wnd;
        uint16_t up;
        uint32_t wl1;
        uint32_t wl2;
    } snd;
    uint32_t iss;
    struct {
        uint32_t nxt;
        uint16_t wnd;
        uint16_t up;
    } rcv;
    uint32_t irs;
    uint16_t mtu;
    uint16_t mss;
    uint8_t buf[65535]; /* receive buffer */
    struct sched_ctx ctx;
    struct queue_head queue; /* retransmit queue */
    struct tcp_pcb *parent;
    struct queue_head backlog;
};

struct udp_pcb {
    int state;
    struct ip_endpoint local;
    struct queue_head queue; /* receive queue */
    struct sched_ctx ctx;
};
```

## 各レイヤのヘッダー定義

```c
// Ethernet
struct ether_hdr {
    uint8_t dst[ETHER_ADDR_LEN];    // 宛先MACアドレス
    uint8_t src[ETHER_ADDR_LEN];    // 送信元MACアドレス
    uint16_t type;                  // タイプ (IPv4: 0x0800, ARP: 0x0806)
};

struct ip_hdr {
    uint8_t vhl;        // [0:3] version, [4-8] ヘッダー長
    uint8_t tos;        // サービスタイプ
    uint16_t total;     // パケット長 (IPヘッダーを含む)
    uint16_t id;        // 識別番号
    uint16_t offset;    // [0:2] フラグ, [3-15] フラグメントオフセット
    uint8_t ttl;        // 生存時間
    uint8_t protocol;   // プロトコル(TCP/UDPなど)
    uint16_t sum;       // ヘッダーチェックサム
    ip_addr_t src;      // 送信元IPアドレス
    ip_addr_t dst;      // 宛先IPアドレス
    uint8_t options[];  // オプション
};

struct tcp_hdr {
    uint16_t src;       // 送信元ポート番号
    uint16_t dst;       // 宛先ポート番号
    uint32_t seq;       // シーケンス番号
    uint32_t ack;       // 確認応答番号
    uint8_t off;        // [0:3] データオフセット, [4-6] 予約
    uint8_t flg;        // off[7]+[0-7] コントロールフラグ
    uint16_t wnd;       // ウィンドウサイズ
    uint16_t sum;       // チェックサム
    uint16_t up;        // 緊急ポインタ
};

struct udp_hdr {
    uint16_t src;       // 送信元ポート番号
    uint16_t dst;       // 宛先ポート番号
    uint16_t len;       // パケット長（ヘッダーを含む）
    uint16_t sum;       // チェックサム
};
```

## 送受信信処理

### TCP送信

- syssocket.c#sys_send(void): call socket_write()
    - socket.c#socket_write(struct socket *s, char *addr, int n): call tcp_send();
        - tcp.c#tcp_send(int id, uint8_t *data, size_t len): call tcp_output()
        - tcp.c#tcp_output(struct tcp_pcb *pcb, uint8_t flg, uint8_t *data, size_t len): call tcp_output_segment()
        - tcp.c#tcp_output_segment(uint32_t seq, uint32_t ack, uint8_t flg, uint16_t wnd,
            uint8_t *data, size_t len, struct ip_endpoint *local, struct ip_endpoint *foreign): call ip_output()
            - ip.c#ip_output(uint8_t protocol, const uint8_t *data, size_t len, ip_addr_t src, ip_addr_t dst): call ip_output_core()
            - ip.c#ip_output_core(struct ip_iface *iface, uint8_t protocol, const uint8_t *data, size_t len, ip_addr_t src, ip_addr_t dst, ip_addr_t nexthop, uint16_t id, uint16_t offset): call ip_output_device()
            - ip.c#ip_output_device(struct ip_iface *iface, const uint8_t *data, size_t len, ip_addr_t dst): call net_device_output()
                - net.c#net_device_output(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst): call dev->ops->transmit();
                    - virtio_net.c#virtio_net_write(struct net_device *dev, const uint8_t *data, size_t len): 送信

### TCP受信

#### 初期化と登録

1. virtio_netで割り込みが発生するとvirtio_net_intr()がcallされる
2. virtio_net_intr()はether_input_helper(virtio_net_read)をcall
3. ether_input_helper()はvirtio_net_read()をcall
4. virtio_net_read()はデータをbufにセットする
5. ether_input_helper()はnet_input_handler()をcallする
6. net_input_handler()はデータをqueueにセットしてINTR_IRQ_SOFTIRQソフト割り込みを発生させる
7. この割り込みによりnet_softirq_handler()がcallされる
8. net_softirq_handler()はip_input()をcallする
9. ip_input()はtcp_input()をcallする
10. tcp_input()は諸々のTCP処理を行う

- trap.c
    - devintr(): if(irq == VIRTIO1_IRQ) virtio_net_intr();
    - softinter(): if (irqs & SOFT_IRQ_NET_RX(=INTR_IRQ_SOFTIRQ)) net_softirq_handler();
- virtio_net.c
    - virtio_net_intr(void)
        call ether_input_helper(, virtio_net_read);
        intr_raise_irq(INTR_IRQ_SOFTIRQ);
    - virtio_net_read(struct net_device *dev, uint8_t *buf, size_t size): bufに受信データをセット
- ether.c
    - ether_input_helper(struct net_device *dev, ether_input_func_t callback)
        call virtio_net_read();
        call net_input_handler()
- net.c
    - net_input_handler(uint16_t type, const uint8_t *data, size_t len, struct net_dev
ice *dev)
        queue_push(&proto->queue, entry);
        intr_raise_irq(INTR_IRQ_SOFTIRQ);
    - net_softirq_handler(void): call proto->handler() = ip_input();
    - netproto_register(type, (*handler)(packet, plen, dev))
- ip.c
    - ip_init(): net_protocol_register(NET_PROTOCOL_TYPE_IP, ip_input)
    - ip_input(const uint8_t *data, size_t len, struct net_device *dev) call proto->handler = tcp_input
- tcp.c
    - tcp_init(): ip_protocol_register(IP_PROTOCOL_TCP, tcp_input)
    - tcp_input(const uint8_t *data, size_t len, ip_addr_t src, ip_addr_t dst, struct ip_iface *iface): call tcp_segment_arrives()
    - tcp_segment_arrives(struct tcp_segment_info *seg, uint8_t flags, uint8_t *data, size_t len, struct ip_endpoint *local, struct ip_endpoint *foreign): 諸々処理


#### 処理

- syscall.c#sys_recv();
  - socket.c#socketread(struct socket *s, char *addr, int n);
    - tcp.c#tcp_api_recv(int soc, uint8_t *buf, size_t size); データが到着するまでsleep
      - e1000intr()から順にデータが設定されていく

### api

```c
int sys_ioctl(void);
int sys_socket(void);
int sys_connect(void);
int sys_bind(void);
int sys_listen(void);
int sys_accept(void);
int sys_recv(void);
int sys_send(void);
int sys_recvfrom(void);
int sys_sendto(void);
```
