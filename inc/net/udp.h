// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_UDP_H
#define INC_NET_UDP_H

#include <net/ip.h>

ssize_t udp_output(struct ip_endpoint *src, struct ip_endpoint *dst, const uint8_t *buf, size_t len);

int udp_init(void);

int udp_open(void);
int udp_close(int id);
int udp_bind(int index, struct ip_endpoint *local);
ssize_t udp_sendto(int id, uint8_t *buf, size_t len, struct ip_endpoint *foreign);
ssize_t udp_recvfrom(int id, uint8_t *buf, size_t size, struct ip_endpoint *foreign);

#endif
