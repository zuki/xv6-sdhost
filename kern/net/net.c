// From https://github.com/pandax381/xv6-riscv-net
#include <types.h>
#include <config.h>
#include <net/util.h>
#include <net/net.h>
#include <net/if.h>
#include <net/ip.h>
#include <net/platform.h>
#include <linux/time.h>
#include <mm.h>
#include <timer.h>
#include <clock.h>
#include <console.h>
#include <string.h>

struct net_protocol {
    struct net_protocol *next;
    uint16_t type;                  // NET_PROTOCOL_TYPE_XXX: IP, ARP, IPv6
    struct queue_head queue;        /* input queue */
    void (*handler)(const uint8_t *data, size_t len, struct net_device *dev);
};

struct net_protocol_queue_entry {
    struct net_device   *dev;
    size_t              len;
    uint8_t             data[];
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

/* NOTE: if you want to add/delete the entries after net_run(), you need to protect these lists with a mutex. */
static struct net_device *devices;
static struct net_protocol *protocols;
//static struct net_timer *timers;
static struct net_event *events;
static mutex_t mutex;

static char *net_dev_type_str[5] = {
    "DUMMY",
    "LOOPBAC",
    "ETHERNET",
    "WLAN",
    "ANY"
};

struct net_device *net_device_alloc(void)
{
    struct net_device *dev;

    dev = memory_alloc(sizeof(*dev));
    if (!dev) {
        error("memory_alloc() failure");
        return NULL;
    }
    memset(dev, 0, sizeof(*dev));
    return dev;
}

/* NOTE: must not be call after net_run() */
int net_device_register(struct net_device *dev)
{
    dev->next = devices;
    devices = dev;
    info("dev=%s, type=%d (%s)", dev->name, dev->type, net_dev_type_str[dev->type]);
    return 0;
}

int net_device_open(struct net_device *dev)
{
    if (NET_DEVICE_IS_UP(dev)) {
        error("already opened, dev=%s", dev->name);
        return -1;
    }
    if (dev->ops->open) {
        if (dev->ops->open(dev) == -1) {
            error("failure, dev=%s", dev->name);
            return -1;
        }
    }
    dev->flags |= IFF_UP;
    info("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
    return 0;
}

int net_device_close(struct net_device *dev)
{
    if (!NET_DEVICE_IS_UP(dev)) {
        error("not opened, dev=%s", dev->name);
        return -1;
    }
    if (dev->ops->close) {
        if (dev->ops->close(dev) == -1) {
            error("failure, dev=%s", dev->name);
            return -1;
        }
    }
    dev->flags &= ~IFF_UP;
    info("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
    return 0;
}

struct net_device *net_device_by_index(unsigned int index)
{
    struct net_device *entry;

    for (entry = devices; entry; entry = entry->next) {
        if (entry->index == index) {
            break;
        }
    }
    return entry;
}

struct net_device *net_device_by_name(const char *name)
{
    struct net_device *entry;

    for (entry = devices; entry; entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            break;
        } else {
            trace("name: %s, dev->name: %s", name, entry->name);
        }
    }
    return entry;
}

/* NOTE: must not be call after net_run() */
int net_device_add_iface(struct net_device *dev, struct net_iface *iface)
{
    struct net_iface *entry;
    trace("dev->ifaces: 0x%p", dev->ifaces);
    for (entry = dev->ifaces; entry; entry = entry->next) {
        if (entry->family == iface->family) {
            /* NOTE: For simplicity, only one iface can be added per family. */
            error("already exists, dev=%s, family=%d", dev->name, entry->family);
            return -1;
        }
    }
    iface->next = dev->ifaces;
    iface->dev = dev;
    dev->ifaces = iface;
    return 0;
}

struct net_iface *net_device_get_iface(struct net_device *dev, int family)
{
    struct net_iface *entry;

    for (entry = dev->ifaces; entry; entry = entry->next) {
        if (entry->family == family) {
            break;
        }
    }
    return entry;
}

int net_device_output(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst)
{
    if (!NET_DEVICE_IS_UP(dev)) {
        error("not opened, dev=%s", dev->name);
        return -1;
    }
    if (len > dev->mtu) {
        error("too long, dev=%s, mtu=%u, len=%llu", dev->name, dev->mtu, len);
        return -1;
    }
    trace("dev=%s, type=0x%04x, len=%llu", dev->name, type, len);
    debugdump(data, len, "net_device_output");
    trace("transmit: 0x%p, len: %d", dev->ops->transmit, len);
    if (dev->ops->transmit(dev, type, data, len, dst) == -1) {
        error("device transmit failure, dev=%s, len=%u", dev->name, len);
        return -1;
    }
    return 0;
}

/* NOTE: must not be call after net_run() */
int net_protocol_register(uint16_t type, void (*handler)(const uint8_t *data, size_t len, struct net_device *dev))
{
    struct net_protocol *proto;

    for (proto = protocols; proto; proto = proto->next) {
        if (type == proto->type) {
            error("already registered, type=0x%04x", type);
            return -1;
        }
    }
    proto = memory_alloc(sizeof(*proto));
    if (!proto) {
        error("memory_alloc() failure");
        return -1;
    }
    proto->type = type;
    proto->handler = handler;
    proto->next = protocols;
    queue_init(&proto->queue);
    trace("proc queue: type: %d, queue: %p", type, &proto->queue);
    protocols = proto;
    info("type=0x%04x (%s)", type, type == 0x0800 ? "IP" : type == 0x0806 ? "ARP" : "IPv6");
    return 0;
}

/* NOTE: must not be call after net_run() */
void net_timer_register(uint64_t expires, fn handler, void *param, void *context) {
    struct timer_list *timer = (struct timer_list *)alloc_timer();
    uint64_t *params = (uint64_t *)kmalloc(2 * sizeof(uint64_t));
    params[0] = (uint64_t)param;
    params[1] = (uint64_t)context;
    timer->expires = expires;
    timer->data = (uint64_t)params;
    timer->fn = handler;
    add_timer(timer);
}

#if 0
int net_timer_register(struct timeval interval, void (*handler)(void))
{
    struct net_timer *timer;

    timer = memory_alloc(sizeof(*timer));
    if (!timer) {
        error("memory_alloc() failure");
        return -1;
    }
    timer->interval = interval;
    gettimeofday(&timer->last, NULL);
    timer->handler = handler;
    timer->next = timers;
    timers = timer;
    info("registered: interval={%d, %d}", interval.tv_sec, interval.tv_usec);
    return 0;
}

int net_timer_handler(void)
{
    struct net_timer *timer;
    struct timeval now, diff;

    for (timer = timers; timer; timer = timer->next) {
        gettimeofday(&now, NULL);
        timersub(&now, &timer->last, &diff);
        if (timercmp(&timer->interval, &diff, <) != 0) { /* true (!0) or false (0) */
            timer->handler();
            timer->last = now;
        }
    }
    return 0;
}
#endif

// ether.c#ether_input_helper(dev, callback)から呼ばれる
// 成功したら 0, エラーの場合は -1
int net_input_handler(uint16_t type, const uint8_t *data, size_t len, struct net_device *dev)
{
    struct net_protocol *proto;
    struct net_protocol_queue_entry *entry;

    mutex_lock(&mutex);
    for (proto = protocols; proto; proto = proto->next) {
        if (proto->type == type) {
            entry = memory_alloc(sizeof(*entry) + len);
            if (!entry) {
                error("memory_alloc() failure");
                mutex_unlock(&mutex);
                return -1;
            }
            entry->dev = dev;
            entry->len = len;
            memcpy(entry->data, data, len);
            if (!queue_push(&proto->queue, entry)) {
                error("queue_push() failure");
                memory_free(entry);
                mutex_unlock(&mutex);
                return -1;
            }
            trace("queue pushed (num:%u), dev=%s, type=0x%04x, len=%llu", proto->queue.num, dev->name, type, len);
            //debugdump(data, len, "net_input_data");
            intr_raise_irq(INTR_IRQ_SOFTIRQ);
            mutex_unlock(&mutex);
            return 0;
        }
    }
    /* unsupported protocol */
    //debug("unsupported protocol: %u", type);
    mutex_unlock(&mutex);
    return 0;
}

int net_softirq_handler(void)
{
    struct net_protocol *proto;
    struct net_protocol_queue_entry *entry;

    mutex_lock(&mutex);
    for (proto = protocols; proto; proto = proto->next) {
        while (1) {
            entry = queue_pop(&proto->queue);
            if (!entry) {
                break;
            }
            trace("queue popped (num:%u), dev=%s, type=0x%04x, len=%llu", proto->queue.num, entry->dev->name, proto->type, entry->len);
            debugdump(entry->data, entry->len, "queue entry");
            proto->handler(entry->data, entry->len, entry->dev);
            memory_free(entry);
        }
    }
    mutex_unlock(&mutex);
    return 0;
}

/* NOTE: must not be call after net_run() */
int net_event_subscribe(void (*handler)(void *arg), void *arg)
{
    struct net_event *event;

    event = memory_alloc(sizeof(*event));
    if (!event) {
        error("memory_alloc() failure");
        return -1;
    }
    event->handler = handler;
    event->arg = arg;
    event->next = events;
    events = event;
    return 0;
}

int net_event_handler(void)
{
    struct net_event *event;

    for (event = events; event; event = event->next) {
        event->handler(event->arg);
    }
    return 0;
}

void net_raise_event()
{
    intr_raise_irq(INTR_IRQ_EVENT);
}

void set_ip_config(struct net_device *dev)
{
    //char addr[256];

    struct ip_iface *iface = ip_iface_alloc(LOCAL_IP_ADDR, NETMASK);
    assert(iface != 0);

    trace("dev: 0x%p, iface: 0x%p, unicast: %s, netmask: %s, broadcast: %s",
        dev, iface, ip_addr_ntop(iface->unicast, addr, 256), ip_addr_ntop(iface->netmask, addr, 256),
        ip_addr_ntop(iface->broadcast, addr, 256));
    ip_iface_register(dev, iface);
    ip_route_set_default_gateway(iface, DEFAULT_GATEWAY);
}

static int netrun(void)
{
    struct net_device *dev;

    if (intr_run() == -1) {
        error("intr_run() failure");
        return -1;
    }
    trace("open all devices...");
    for (dev = devices; dev; dev = dev->next) {
        debug("open %s", dev->name);
        net_device_open(dev);
        //while (!dev->ops->linkup(dev)) {
        //    delay(1);
        //}
    }

    info("running...");
    return 0;
}

void net_shutdown(void)
{
    struct net_device *dev;

    trace("close all devices...");
    for (dev = devices; dev; dev = dev->next) {
        net_device_close(dev);
    }
    intr_shutdown();
    info("shutting down");
}

#include <net/ip.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>

static int netinit(void)
{
    mutex_init(&mutex, "net_mutex");

    if (intr_init() == -1) {
        error("intr_init() failure");
        return -1;
    }

    if (ip_init() == -1) {
        error("ip_init() failure");
        return -1;
    }
    if (arp_init() == -1) {
        error("arp_init() failure");
        return -1;
    }
    if (icmp_init() == -1) {
        error("icmp_init() failure");
        return -1;
    }
    if (udp_init() == -1) {
        error("udp_init() failure");
        return -1;
    }
    if (tcp_init() == -1) {
        error("tcp_init() failure");
        return -1;
    }

    return 0;
}



void net_init(void)
{
    if (netinit() == -1) {
        panic("net_init() failure");
    }
    //struct net_device *dev = net_device_by_index(NET_DRV);
    //set_ip_config(dev);
    info("net_init ok");
}

void net_run(void)
{
    if (netrun() == -1) {
        panic("net_run() failure");
    }
}
