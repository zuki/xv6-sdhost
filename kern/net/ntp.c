#include <types.h>
#include <config.h>
#include <console.h>
#include <mm.h>
#include <clock.h>
#include <string.h>
#include <net/ntp.h>
#include <net/dns.h>
#include <net/udp.h>
#include <net/util.h>

int ntp_get_time(void) {
    ip_addr_t ntp_ip;
    struct ip_endpoint local, peer;
    int errnum = -1;
    char *resbuf = 0;

    // 1. DNSサーバのipアドレスを取得
    if (dns_resolve(NTP_SERVER, &ntp_ip) < 0) {
        error("could not get ntp_ip");
        return -1;
    }
#if 0
    char name[64];
    ip_addr_ntop(ntp_ip, name, 64);
    debug("ntp_ip %s", name);
#endif

    // 2. udpをopen
    int udp_id = udp_open();
    if (udp_id < 0) {
        error("could not open udp");
        return udp_id;
    }
    trace("udp_id: %d", udp_id);

    // 2. ロカールIPアドレス/portとDNSサーバのIP/portを用意
    ip_addr_pton(LOCAL_IP_ADDR, &local.addr);
    local.port = BE(NTP_PORT);

    peer.addr = ntp_ip;
    peer.port = BE(NTP_PORT);

    // 3. ローカルIPをudpにbind
    if (udp_bind(udp_id, &local) < 0) {
        error("could not bind udp");
        goto err;
    }
    trace("bind ok");

    // 4. DNSリクエストパケットをreqbufに作成してNTPパケットをセットする
    ntp_t ntp_q;
    memset(&ntp_q, 0, sizeof(ntp_t));
    ntp_q.flags = NTP_FLAG;
    ntp_q.poll = NTP_POLL;

    //hexdump((const void *)&ntp_q, sizeof(ntp_t), "ntp_q");

    // 5. 受信用バッファを用意する
    resbuf = (char *)kmalloc(MAX_NTP_PACKET);
    assert(resbuf != 0);
    ssize_t reslen;
    uint32_t try = 1;
    do {
        // 6. DNSリクエストを送信する
        if (try++ > 3 || udp_sendto(udp_id, (uint8_t *)&ntp_q, sizeof(ntp_t), &peer) < 0) {
            error("try out or failed udp_sendto");
            goto err;
        }
        trace("[%d] sendto ok", try - 1);
        // 7. レスポンスが来るまで1秒待つ: 待ち時間は要検討
        delay(1);
        // 8. レスポンスを受信する
        reslen = udp_recvfrom(udp_id, (uint8_t *)resbuf, MAX_NTP_PACKET, &peer);
        if (reslen < 0) {
            error("udp_recvfrom failed");
            goto err;
        }
        trace("[%d] udp_recvfrom reslen: %d", try - 1, reslen);
    } while (reslen < (int)(sizeof(ntp_t)));

    //hexdump((const void *)resbuf, reslen, "ntp");

    // 9. レスポンスをパースする
    ntp_t *ntp = (ntp_t *)resbuf;
    trace("timestamp: 0x%llx, ntoh: 0x%llx", ntp->rcv_timestamp, ntoh64(ntp->rcv_timestamp));
    trace("vu: 0x%x, vl: 0x%x", (uint32_t)((ntp->rcv_timestamp & 0xffffffff00000000) >> 32), (uint32_t)(ntp->rcv_timestamp & 0xffffffff))
    uint64_t timestamp = ntoh64(ntp->rcv_timestamp);
    uint32_t sec = (uint32_t)((timestamp & 0xffffffff00000000) >> 32);
    uint32_t utc = sec - UNIX_EPOC;
    trace("timestamp: 0x%x, utc: %d", timestamp, utc);
    if (utc > 0) {
        struct timespec tp;
        tp.tv_sec = (time_t)utc;
        tp.tv_nsec = 0;
        if (clock_settime(CLOCK_REALTIME, &tp) < 0) {
            error("failed clock_settime: utc: %d", utc);
        }
    }

err:
    udp_close(udp_id);
    if (resbuf != 0)
        kmfree(resbuf);
    return errnum;
}
