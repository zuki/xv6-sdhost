// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NEt_SOCET_H
#define INC_NEt_SOCET_H

#include <types.h>
#include <net/sockio.h>

#define PF_INET     2

#define AF_INET     PF_INET

#define SOCK_DGRAM  2
#define SOCK_STREAM 1

#define IPPROTO_UDP 17
#define IPPROTO_TCP 6

#define INADDR_ANY  ((uint32_t)0)

struct in_addr {
    uint32_t s_addr;
};

struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};

struct sockaddr_in {
    unsigned short sin_family;      //
    uint16_t sin_port;
    struct in_addr sin_addr;
};

#define IFNAMSIZ    16

struct ifreq {
    char ifr_name[IFNAMSIZ]; /* インタフェース名 */
    union {
        struct sockaddr ifr_addr;               // 読み出し元アドレス
        struct sockaddr ifr_dstaddr;            // 宛先アドレス
        struct sockaddr ifr_broadaddr;          // ブロードキャストアドレス
        struct sockaddr ifr_netmask;            // サブネットマスク
        struct sockaddr ifr_hwaddr;
        short           ifr_flags;
        int             ifr_ifindex;
        int             ifr_metric;
        int             ifr_mtu;
//      struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char           *ifr_data;
    };
};

struct socket {
    int type;
    int desc;
};

int         socket_alloc(int domain, int type, int protocol);
int         socket_close(struct socket *s);
int         socket_connect(struct socket *s, struct sockaddr *addr, int addrlen);
int         socket_bind(struct socket *s, struct sockaddr *addr, int addrlen);
int         socket_listen(struct socket *s, int backlog);
int         socket_accept(struct socket *s, struct sockaddr *addr, int *addrlen);
int         socket_read(struct socket *s, char *buf, int n);
int         socket_write(struct socket *s, char *buf, int n);
int         socket_recvfrom(struct socket *s, char *buf, int n, struct sockaddr *addr, int *addrlen);
int         socket_sendto(struct socket *s, char *buf, int n, struct sockaddr *addr, int addrlen);
int         socket_ioctl(struct socket *s, int req, void *arg);

#endif
