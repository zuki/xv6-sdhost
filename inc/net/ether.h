// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_ETHER_H
#define INC_NET_ETHER_H

#include <net/net.h>
#include <types.h>

#define ETHER_ADDR_LEN          6
#define ETHER_ADDR_STR_LEN      18 /* "xx:xx:xx:xx:xx:xx\0" */

#define ETHER_HDR_SIZE          14
#define ETHER_FRAME_SIZE_MIN    60 /* without FCS */
#define ETHER_FRAME_SIZE_MAX    1514 /* without FCS */
#define ETHER_PAYLOAD_SIZE_MIN  (ETHER_FRAME_SIZE_MIN - ETHER_HDR_SIZE)
#define ETHER_PAYLOAD_SIZE_MAX  (ETHER_FRAME_SIZE_MAX - ETHER_HDR_SIZE)

/* see https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.txt */
#define ETHER_TYPE_IP           0x0800
#define ETHER_TYPE_ARP          0x0806
#define ETHER_TYPE_IPV6         0x86dd
#define ETHER_TYPE_EAPOL        0x888e
#define ETHER_TYPE_PRE_AUTH     0x88c7

// Ethernetヘッダー構造体
struct ether_hdr {
    uint8_t dst[ETHER_ADDR_LEN];
    uint8_t src[ETHER_ADDR_LEN];
    uint16_t type;
};

extern const uint8_t ETHER_ADDR_ANY[ETHER_ADDR_LEN];
extern const uint8_t ETHER_ADDR_BROADCAST[ETHER_ADDR_LEN];

int ether_addr_pton(const char *p, uint8_t *n);
char *ether_addr_ntop(const uint8_t *n, char *p, size_t size);

typedef ssize_t (*ether_transmit_func_t)(struct net_device *dev, const uint8_t *data, size_t len);
typedef ssize_t (*ether_input_func_t)(struct net_device *dev, uint8_t *buf, size_t size);

int ether_transmit_helper(struct net_device *dev, uint16_t type, const uint8_t *payload, size_t plen, const void *dst, ether_transmit_func_t callback);
int ether_input_helper(struct net_device *dev, ether_input_func_t callback);
void ether_setup_helper(struct net_device *dev);

#endif
