#ifndef INC_NET_DNS_H
#define INC_NET_DNS_H

#include <types.h>
#include <net/ip.h>

#define MAX_HOSTNAME_SIZE       256
#define DNS_MAX_MESSAGE_SIZE    512

#define DNS_FLAGS_QR        0x8000
#define DNS_FLAGS_OPCODE    0x7800
    #define DNS_FLAGS_OPCODE_QUERY      0x0000
    #define DNS_FLAGS_OPCODE_IQUERY     0x0800
    #define DNS_FLAGS_OPCODE_STATUS     0x1000
#define DNS_FLAGS_AA        0x0400
#define DNS_FLAGS_TC        0x0200
#define DNS_FLAGS_RD        0x0100
#define DNS_FLAGS_RA        0x0080
#define DNS_FLAGS_RCODE     0x000F
    #define DNS_RCODE_SUCCESS           0x0000
    #define DNS_RCODE_FORMAT_ERROR      0x0001
    #define DNS_RCODE_SERVER_FAILURE    0x0002
    #define DNS_RCODE_NAME_ERROR        0x0003
    #define DNS_RCODE_NOT_IMPLEMENTED   0x0004
    #define DNS_RCODE_REFUSED           0x0005
#define DNS_NAME_FLAG       0xC0
    #define DNS_NAME_FLAG_UNPACK        0x00
    #define DNS_NAME_FLAG_PACKED        0xC0

#define DNS_FLAGS_QUERY_MASK      (DNS_FLAGS_QR | DNS_FLAGS_OPCODE | DNS_FLAGS_TC | DNS_FLAGS_RCODE)
#define DNS_FLAGS_QUERY_OK  (DNS_FLAGS_QR | DNS_FLAGS_OPCODE_QUERY | DNS_RCODE_SUCCESS)
#define DNS_QTYPE_A         1
#define DNS_QCLASS_IN       1
#define DNS_RDLENGTH_AIN    4

#define DNS_PORT    53

// |--+--------+--+--+--+--+--+--+--+--------|
// |                ID                       |
// |QR| Opcode |AA|TC|RD|RA| Z|AD|CD|  RCODE |
// |             QDCOUNT                     |
// |             ANCOUNT                     |
// |             NSCOUNT                     |
// |             ARCOUNT                     |
// | NAME (3www6google3com\0)      |TYPE|CLAS|
//
// |NAME|TYPE|CLAS|   TTL    | LEN| IP ADDR  |

// NAME: [15:14] 00: 通常のNAME,
//               11: [13:0]がNAMEのあるパケット先頭からのOFSSET

typedef struct dns_hdr {
    uint16_t id;
    uint16_t flag;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} PACKED dns_hdr_t;

typedef struct dns_query {
    uint16_t qtype;
    uint16_t qclass;
} PACKED dns_query_t;

typedef struct dns_rr {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t  rdata[DNS_RDLENGTH_AIN];
} PACKED dns_rr_t;

#define DNS_RR_HDR_LEN  (sizeof (dns_rr_t) - DNS_RDLENGTH_AIN)

int dns_resolve(const char *host, ip_addr_t *ipaddr);

#endif
