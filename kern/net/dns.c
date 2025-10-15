#include <types.h>
#include <net/dns.h>
#include <net/ip.h>
#include <net/net.h>
#include <net/udp.h>
#include <net/util.h>
#include <arm.h>
#include <string.h>
#include <config.h>
#include <console.h>
#include <mm.h>

static uint16_t next_id = 1;
static uint16_t port = 53000;

static void dns_dump(const uint8_t *data, size_t len, size_t dst_len)
{
#ifdef LOG_DEBUG
    dns_hdr_t *message;
    dns_query_t *query;

    message = (dns_hdr_t *)data;
    cprintf("=== DNS dump ===\n");
    cprintf("        id: %u\n", ntoh16(message->id));
    cprintf("      flag: 0x%08x\n", ntoh16(message->flag));
    cprintf("   qdcount: %u\n", ntoh16(message->qdcount));
    cprintf("   ancount: %u\n", ntoh16(message->ancount));
    cprintf("   nscount: %u\n", ntoh16(message->nscount));
    cprintf("   arcount: %u\n", ntoh16(message->arcount));
    if (dst_len > 0) {
        cprintf("      dist: ");
        for (int i = 0; i < dst_len; i++) {
            cprintf("%02x ", data[sizeof(dns_hdr_t) + i]);
        }
        cprintf("\n");
        query = (dns_query_t *)((size_t)data + sizeof(dns_hdr_t) + dst_len);
        cprintf("     qtype: %u\n", ntoh16(query->qtype));
        cprintf("    qclass: %u\n", ntoh16(query->qclass));
    }

    hexdump(data, len, "DNS");
    cprintf("\n");
#endif
}

int dns_resolve(const char *host, ip_addr_t *ipaddr)
{
    struct ip_endpoint local, peer;
    int errnum = -1;
    char *buffer = 0;
    char *name = 0;
    char *recvbuf = 0;

    assert(host != 0);
    debug("resolve %s", host);
    // ホスト名がIPアドレスの場合はip_addr_tに変換する
    if ('1' <= *host && *host <= '9') {
        return ip_addr_pton(host, ipaddr);
    }

    int udp_id = udp_open();
    if (udp_id < 0) {
        error("could not open udp");
        return udp_id;
    }
    trace("udp_id: %d", udp_id);

    ip_addr_pton(LOCAL_IP_ADDR, &local.addr);
    local.port = ntoh16(port++);

    ip_addr_pton(DNS_SERVER, &peer.addr);
    peer.port = BE(DNS_PORT);
    if (udp_bind(udp_id, &local) < 0) {
        error("could not bind udp");
        goto err;
    }
    trace("bind ok");

    buffer = (char *)kmalloc(DNS_MAX_MESSAGE_SIZE);
    assert(buffer != 0);
    memset(buffer, 0, DNS_MAX_MESSAGE_SIZE);
    dns_hdr_t *dns_hdr = (dns_hdr_t *)buffer;

    uint16_t id = next_id++;
    dns_hdr->id = hton16(id);
    dns_hdr->flag = hton16(DNS_FLAGS_OPCODE_QUERY | DNS_FLAGS_RD);
    dns_hdr->qdcount = hton16(1);
    //dns_dump((const uint8_t *)dns_hdr, sizeof(dns_hdr_t), 0);

    char *query = (char *)(buffer + sizeof(dns_hdr_t));
    name = (char *)kmalloc(MAX_HOSTNAME_SIZE);
    assert(name != 0);
    memset(name, 0, MAX_HOSTNAME_SIZE);
    strncpy(name, host, strlen(host));

    char *save;
    size_t length, dist_len = 0;
    char *label = strtok_r1(name, '.', &save);
    while (label != 0) {
        length = strlen(label);
        if (length > 255) {
            error("too long label %s, length: %d", label, length);
            goto err;
        }
        trace("label: %s, len: %d", label, length);
        *query++ = (char)length;
        strncpy((char *)query, label, length);
        query += length;
        dist_len += length + 1;
        label = strtok_r1(0, '.', &save);
    }
    *query++ = '\0';
    dist_len += 1;

    dns_query_t *dns_query = (dns_query_t *)query;
    dns_query->qtype = BE(DNS_QTYPE_A);
    dns_query->qclass = BE(DNS_QCLASS_IN);
    query += sizeof(dns_query_t);

    int size = (int)(query - buffer);
    assert(size <= DNS_MAX_MESSAGE_SIZE);
    dns_dump((const uint8_t *)buffer, size, dist_len);
#if 1
    recvbuf = (char *)kmalloc(DNS_MAX_MESSAGE_SIZE);
    assert(recvbuf != 0);
    ssize_t recvlen;
    uint32_t try = 1;
    do {
        if (try++ > 3 || udp_sendto(udp_id, (uint8_t *)buffer, size, &peer) < 0) {
            error("try out or failed udp_sendto");
            goto err;
        }
        trace("[%d] sendto ok", try - 1);
        delay(1);
        recvlen = udp_recvfrom(udp_id, (uint8_t *)recvbuf, DNS_MAX_MESSAGE_SIZE, &peer);
        if (recvlen < 0) {
            error("udp_recvfrom failed");
            goto err;
        }
        trace("[%d] udp_recvfrom recvlen: %d", try - 1, recvlen);
    } while (recvlen < (int)(sizeof(dns_hdr_t) + DNS_RR_HDR_LEN));

    dns_hdr = (dns_hdr_t *)recvbuf;
    if (dns_hdr->id != hton16(id)
        || (dns_hdr->flag & BE(DNS_FLAGS_QUERY_MASK)) != BE(DNS_FLAGS_QUERY_OK)
        || dns_hdr->qdcount != BE(1) || dns_hdr->ancount == BE(0)) {
        error("wrong response: id: 0x%x, flags: 0x%d, qdcount: %d, ancount: %d",
                dns_hdr->id, dns_hdr->flag, dns_hdr->qdcount, dns_hdr->ancount);
        goto err;
    }

    char *response = recvbuf + sizeof(dns_hdr_t);
    hexdump((const char *)recvbuf, DNS_MAX_MESSAGE_SIZE, "DNS Response");

    while ((length = *response++) > 0) {
        response += length;
        if (response - recvbuf >= recvlen) {
            error("too long length 1: %d, recvlen: %d", response - recvbuf, recvlen);
            goto err;
        }
    }

    response += sizeof(dns_query_t);
    if (response - recvbuf >= recvlen) {
        error("too long length 2: %d, recvlen: %d", response - recvbuf, recvlen);
        goto err;
    }

    dns_rr_t rr;

    // 回答をパース
    while (1) {
        length = *response++;
        if ((length & 0xc0) == 0xc0) {
            response++;
        } else {
            do {
                response += length;
                if (response - recvbuf >= recvlen) {
                    error("too long length 3: %d, recvlen: %d", response - recvbuf, recvlen);
                    goto err;
                }
            } while ((length = *response++) > 0);
        }

        if (response - recvbuf > (int) (recvlen - sizeof(dns_rr_t))) {
            error("response - recvbuf > recsize - sizeof: %d", response - recvbuf);
            goto err;
        }

        memcpy(&rr, response, sizeof(dns_rr_t));
        if (rr.type     == BE(DNS_QTYPE_A) && rr.class == BE(DNS_QCLASS_IN)
         && rr.rdlength == BE(DNS_RDLENGTH_AIN))
            break;

        response += DNS_RR_HDR_LEN + BE(rr.rdlength);
        if (response - recvbuf >= recvlen) {
            error("too long length 4: %d, recvlen: %d", response - recvbuf, recvlen);
            goto err;
        }
    }

    assert(ipaddr != 0);
    *ipaddr = *((ip_addr_t *)rr.rdata);
    ip_addr_ntop(*ipaddr, name, 64);
    debug("resolved addr is %s", name);
#endif
    errnum = 0;

err:
    udp_close(udp_id);
    if (buffer != 0)
        kmfree(buffer);
    if (recvbuf != 0)
        kmfree(recvbuf);
    if (name != 0)
        kmfree(name);

    return errnum;
}
