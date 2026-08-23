// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_ARP_H
#define INC_NET_ARP_H

#include <net/net.h>
#include <net/ip.h>
#include <net/platform.h>

#define ARP_RESOLVE_ERROR      -1
#define ARP_RESOLVE_INCOMPLETE  0
#define ARP_RESOLVE_FOUND       1

extern struct arp_cache *arp_cachep;
extern mutex_t arp_mutex;

int arp_resolve(struct net_iface *iface, ip_addr_t pa, uint8_t *ha);

int arp_init(void);

#endif
