// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_NET_H
#define INC_NET_NET_H

#include <types.h>
#include <linux/time.h>
#include <net/if.h>

#ifndef IFNAMSIZ
#define IFNAMSIZ    16
#endif

#define NET_DEVICE_TYPE_DUMMY       0x0000
#define NET_DEVICE_TYPE_LOOPBACK    0x0001
#define NET_DEVICE_TYPE_ETHERNET    0x0002
#define NET_DEVICE_TYPE_WLAN        0x0003
#define NET_DEVICE_TYPE_ANY         0x0004

// <net/if.h>のIFF_*を使用する
#if 0
#define NET_DEVICE_FLAG_UP          0x0001
#define NET_DEVICE_FLAG_BROADCAST   0x0002
#define NET_DEVICE_FLAG_LOOPBACK    0x0008
#define NET_DEVICE_FLAG_P2P         0x0010
#define NET_DEVICE_FLAG_NEED_ARP    0x0080
#endif

#define NET_DEVICE_ADDR_LEN         6

#define NET_DEVICE_IS_UP(x)     ((x)->flags & IFF_UP)
#define NET_DEVICE_STATE(x)     (NET_DEVICE_IS_UP(x) ? "up" : "down")

/* NOTE: use same value as the Ethernet types */
#define NET_PROTOCOL_TYPE_IP    0x0800
#define NET_PROTOCOL_TYPE_ARP   0x0806
#define NTT_PROTOCOL_TYPE_IPV6  0x86dd

#define NET_IFACE_FAMILY_IP     1
#define NET_IFACE_FAMILY_IPV6   2

#define NET_IFACE(x) ((struct net_iface *)(x))

typedef enum net_index {
    NET_INDEX_LAN7800,
    NET_INDEX_CDC,
    NET_INDEX_BCM4343,
    NET_INDEX_NUMS
} net_index_t;

struct net_device {
    struct net_device *next;
    struct net_iface *ifaces; /* NOTE: net_run()後に追加/削除する場合はmutexでifacesを守る必要がある */
    unsigned int index;
    char name[IFNAMSIZ];
    uint16_t type;          // NET_DEVICE_TYPE_XXX
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
    int (*transmit)(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst);
    int (*linkup)(struct net_device *dev);
};

struct net_iface {
    struct net_iface *next;
    struct net_device *dev; /* back pointer to parent */
    int family;             // IP, IPv6
    /* depends on implementation of protocols. */
};

struct net_device *net_device_alloc(void);
int net_device_open(struct net_device *dev);
int net_device_close(struct net_device *dev);
int net_device_register(struct net_device *dev);
struct net_device *net_device_by_index(unsigned int index);
struct net_device *net_device_by_name(const char *name);
int net_device_add_iface(struct net_device *dev, struct net_iface *iface);
struct net_iface *net_device_get_iface(struct net_device *dev, int family);
int net_device_output(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst);

int net_protocol_register(uint16_t type, void (*handler)(const uint8_t *data, size_t len, struct net_device *dev));

void net_timer_register(uint64_t expires, fn handler, void *param, void *context);

int net_input_handler(uint16_t type, const uint8_t *data, size_t len, struct net_device *dev);
int net_softirq_handler(void);

int net_event_subscribe(void (*handler)(void *arg), void *arg);
int net_event_handler(void);
void net_raise_event(void);

void net_run(void);
void net_shutdown(void);
void net_init(void);

#endif
