// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_TCP_H
#define INC_NET_TCP_H

#include <net/ip.h>

int tcp_init(void);
int tcp_open_rfc793(struct ip_endpoint *local, struct ip_endpoint *foreign, int active);
int tcp_close(int id);
int tcp_open(void);
int tcp_bind(int id, struct ip_endpoint *local);
int tcp_connect(int id, struct ip_endpoint *foreign);
int tcp_listen(int id, int backlog);
int tcp_accept(int id, struct ip_endpoint *foreign);
ssize_t tcp_send(int id, uint8_t *data, size_t len);
ssize_t tcp_receive(int id, uint8_t *buf, size_t size);

#endif
