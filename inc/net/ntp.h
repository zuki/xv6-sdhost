#ifndef INC_NET_NTP_H
#define INC_NET_NTP_H

#include <types.h>

typedef struct ntp {
    uint8_t     flags;
    uint8_t     stratum;
    uint8_t     poll;
    uint8_t     precision;
    uint32_t    root_delay;
    uint32_t    root_dispersion;
    uint32_t    ref_id;
    uint64_t    ref_timestamp;
    uint64_t    org_timestamp;
    uint64_t    rcv_timestamp;
    uint64_t    trans_timestamep;
} ntp_t;

#define UNIX_EPOC   2208988800U
#define NTP_SERVER  "ntp.jst.mfeed.ad.jp"
#define NTP_PORT    123

#define NTP_FLAG        0x23    // LI=0, VN=4, Mode=3
#define NTP_POLL        10

#define MAX_NTP_PACKET  512

int ntp_get_time(void);

#endif
