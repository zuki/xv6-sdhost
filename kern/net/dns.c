#include <types.h>
#include <config.h>
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

static void dns_dump(const uint8_t *data, size_t len, size_t name_len)
{
#ifdef LOG_TRACE
    dns_hdr_t *message;
    dns_query_t *query;
    dns_rr_t *response;
    char addr[64];
    size_t size = 0;

    message = (dns_hdr_t *)data;
    cprintf("=== DNS Packet ===\n");
    cprintf("        id: %u\n", ntoh16(message->id));
    cprintf("      flag: 0x%04x\n", message->flag);
    cprintf("   qdcount: %u\n", ntoh16(message->qdcount));
    cprintf("   ancount: %u\n", ntoh16(message->ancount));
    cprintf("   nscount: %u\n", ntoh16(message->nscount));
    cprintf("   arcount: %u\n", ntoh16(message->arcount));
    if (name_len > 0) {
        cprintf("      dist: ");
        for (int i = 0; i < name_len; i++) {
            char c = data[sizeof(dns_hdr_t) + i];
            if (0x20 <= c && c <= 0x7e)
                cprintf("%c", c);
            else
                cprintf("'%x'", c);
        }
        cprintf("\n");
        query = (dns_query_t *)((size_t)data + sizeof(dns_hdr_t) + name_len);
        cprintf("     qtype: %u\n", ntoh16(query->qtype));
        cprintf("    qclass: %u\n", ntoh16(query->qclass));
    }

    size_t req_len = sizeof(dns_hdr_t) + name_len + sizeof(dns_query_t);
    size += req_len;

    if (len > req_len) {
        size_t rr_len = 0;
        for (int i = 0; i < ntoh16(message->ancount); i++) {
            if ((data[req_len] & DNS_NAME_FLAG) == DNS_NAME_FLAG_PACKED) {
                cprintf("      name: 0x%04x\n", hton16(*((uint16_t *)(data + req_len))));
                response = (dns_rr_t *)(data + size + 2);
                size += 2 + sizeof(dns_rr_t);
            } else {
                response = (dns_rr_t *)(data + size + name_len);
                size += name_len + sizeof(dns_rr_t);
            }
            cprintf("      type: %u\n", ntoh16(response->type));
            cprintf("     class: %u\n", ntoh16(response->class));
            cprintf("       ttl: %u\n", ntoh32(response->ttl));
            cprintf("       len: %u\n", ntoh16(response->rdlength));
            cprintf("        ip: %s\n", ip_addr_ntop(*(ip_addr_t *)(response->rdata), addr, 64));
        }
    }

    hexdump(data, size, "DNS");
    cprintf("\n");
#endif
}

static ssize_t make_query_name(char *query, const char *host) {
    char name[MAX_HOSTNAME_SIZE];

    memset(name, 0, MAX_HOSTNAME_SIZE);
    strncpy(name, host, strlen(host));

    char *save;
    size_t wlen, name_len = 0;
    char *word = strtok_r(name, ".", &save);
    while (word != 0) {
        wlen = strlen(word);
        if (wlen > 255) {
            error("too long word %s, wlen: %d", word, wlen);
            return -1;
        }
        trace("word: %s, wlen: %d", word, wlen);
        *query++ = (char)wlen;
        strncpy((char *)query, word, wlen);
        query += wlen;
        name_len += wlen + 1;
        word = strtok_r(0, ".", &save);
    }
    *query++ = '\0';
    name_len += 1;
    return name_len;
}

int dns_resolve(const char *host, ip_addr_t *ipaddr)
{
    struct ip_endpoint local, peer;
    int errnum = -1;
    char *reqbuf = 0;
    char *resbuf = 0;

    assert(host != 0);
    trace("resolve %s", host);
    // 1. ホスト名がIPアドレスの場合はip_addr_tに変換する
    if ('1' <= *host && *host <= '9') {
        return ip_addr_pton(host, ipaddr);
    }

    // 2. udpをopen
    int udp_id = udp_open();
    if (udp_id < 0) {
        error("could not open udp");
        return udp_id;
    }
    trace("udp_id: %d", udp_id);

    // 2. ロカールIPアドレス/portとDNSサーバのIP/portを用意
    ip_addr_pton(LOCAL_IP_ADDR, &local.addr);
    local.port = ntoh16(port++);

    ip_addr_pton(DNS_SERVER, &peer.addr);
    peer.port = BE(DNS_PORT);

    // 3. ローカルIPをudpにbind
    if (udp_bind(udp_id, &local) < 0) {
        error("could not bind udp");
        goto err;
    }
    trace("bind ok");

    // 4. DNSリクエストパケットをreqbufに作成する
    reqbuf = (char *)kmalloc(DNS_MAX_MESSAGE_SIZE);
    assert(reqbuf != 0);
    memset(reqbuf, 0, DNS_MAX_MESSAGE_SIZE);

    // 4.1 ヘッダー部分をセット
    dns_hdr_t *dns_hdr = (dns_hdr_t *)reqbuf;
    uint16_t id = next_id++;
    dns_hdr->id = hton16(id);
    dns_hdr->flag = hton16(DNS_FLAGS_OPCODE_QUERY | DNS_FLAGS_RD);
    dns_hdr->qdcount = hton16(1);

    // 4.2. DSN解決するホスト名をDNSクエリname形式に変換する: www.google.com -> 0x3www0x6google0x3com0x0
    char *query = (char *)(reqbuf + sizeof(dns_hdr_t));
    ssize_t name_len = make_query_name(query, host);    // name_lenはクエリname開式での長さ
    if (name_len < 0)
        goto err;
    else
        query += name_len;
    // 4.3 クエリタイプとクラスをセット
    dns_query_t *dns_query = (dns_query_t *)query;
    dns_query->qtype = BE(DNS_QTYPE_A);
    dns_query->qclass = BE(DNS_QCLASS_IN);
    query += sizeof(dns_query_t);

    int req_size = (int)(query - reqbuf);       // req_sizeはDNSリクエストパッケージの長さ
    assert(req_size <= DNS_MAX_MESSAGE_SIZE);
    dns_dump((const uint8_t *)reqbuf, req_size, (size_t)name_len);

    // 5. 受信用バッファを用意する
    resbuf = (char *)kmalloc(DNS_MAX_MESSAGE_SIZE);
    assert(resbuf != 0);
    ssize_t reslen;
    uint32_t try = 1;
    do {
        // 6. DNSリクエストを送信する
        if (try++ > 3 || udp_sendto(udp_id, (uint8_t *)reqbuf, req_size, &peer) < 0) {
            error("try out or failed udp_sendto");
            goto err;
        }
        trace("[%d] sendto ok", try - 1);
        // 7. レスポンスが来るまで1秒待つ: 待ち時間は要検討
        delay(1);
        // 8. レスポンスを受信する
        reslen = udp_recvfrom(udp_id, (uint8_t *)resbuf, DNS_MAX_MESSAGE_SIZE, &peer);
        if (reslen < 0) {
            error("udp_recvfrom failed");
            goto err;
        }
        trace("[%d] udp_recvfrom reslen: %d", try - 1, reslen);
    } while (reslen < (int)(sizeof(dns_hdr_t) + DNS_RR_HDR_LEN));
    trace("resbuf: 0x%p, reslen: %d", resbuf, reslen);
    // 9. レスポンスが正しいかチェックする
    dns_hdr = (dns_hdr_t *)resbuf;
    if (dns_hdr->id != hton16(id)   // リクエストクエリに対するレスポインスでない
        || (dns_hdr->flag & BE(DNS_FLAGS_QUERY_MASK)) != BE(DNS_FLAGS_QUERY_OK)     // クエリが失敗
        || dns_hdr->qdcount != BE(1) || dns_hdr->ancount == BE(0)) {                // 回答が0
        error("wrong response: id: 0x%x, flags: 0x%d, qdcount: %d, ancount: %d",
                dns_hdr->id, dns_hdr->flag, dns_hdr->qdcount, dns_hdr->ancount);
        goto err;
    }

    // 10. レスポンスをダンプ出力
    dns_dump((const char *)resbuf, DNS_MAX_MESSAGE_SIZE, name_len);

    // 11. レスポンスをパースする
    // 11.1 リクエスト部分（dns_hdr_t, name, dns_query_t）を読み飛ばす
    char *response = resbuf + req_size;
    trace("response 1: 0x%p, 0x%x", response, *response);
    // 11.2 リクエストNAMEフィールドを読み飛ばす
    // 11.2.1 NAMEは圧縮されている
    if ((*response & DNS_NAME_FLAG) == DNS_NAME_FLAG_PACKED) {
        response += 2;
    // 11.2.2 NAMEはDNSクエリname形式である
    } else {
        char *qname = response;
        while (*qname != '\0') {
            size_t wlen = (size_t)(*qname);
            qname += wlen + 1;
        }
        response = qname + 1;
    }
    // 11.3 クエリTYPEとCLASSを読み飛ばす
    trace("response 2: 0x%p", response);

    // 11.4 回答をパースする
    dns_rr_t *rr;
    for (int i = 0; i < ntoh16(dns_hdr->ancount); i++) {
        rr = (dns_rr_t *)response;
        //memcpy(&rr, response, sizeof(dns_rr_t));
        if (rr->type     != BE(DNS_QTYPE_A)
         || rr->class    != BE(DNS_QCLASS_IN)
         || rr->rdlength != BE(DNS_RDLENGTH_AIN)) {
            response += DNS_RR_HDR_LEN + ntoh16(rr->rdlength);
            if (response - resbuf >= reslen) {
                error("too long length: %d, reslen: %d", response - resbuf, reslen);
                goto err;
            }
            continue;
        }
        assert(ipaddr != 0);
        *ipaddr = *((ip_addr_t *)rr->rdata);
    #if 0
        char name[64];
        ip_addr_ntop(*ipaddr, name, 64);
        debug("resolved addr is %s", name);
    #endif
        errnum = 0;
        break;
    }

err:
    udp_close(udp_id);
    if (reqbuf != 0)
        kmfree(reqbuf);
    if (resbuf != 0)
        kmfree(resbuf);
    return errnum;
}
