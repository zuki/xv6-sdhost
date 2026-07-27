//
// driver_circle.cpp
//
// Driver interface for Circle network driver
// by R. Stange <rsta2@gmx.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Alternatively, this software may be distributed under the terms of BSD
// license.
//

/*
 * The "struct brcmf_*_le" definitions in this file are:
 *
 * Copyright (c) 2012 Broadcom Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <types.h>
#include <utils/common.h>
#include <drivers/driver.h>
#include <utils/eloop.h>

#include <net/net.h>
#include <string.h>
#include <cstring.h>
#include <wlan/bcm4343.h>
#include <console.h>
#include <cstring.h>

// stationドライバ : raspiとwifiルータの間の通信を行う
struct brcmf_bss_info_le {
    uint32_t version;           /* version field */
#define    BRCMF_BSS_INFO_VERSION    109 /* curr ver of brcmf_bss_info_le struct */
    uint32_t length;            /* この構造体のデータのバイト長。versionに始まりIEを含む
                                 */
    uint8_t BSSID[ETH_ALEN];
    uint16_t beacon_period;     /* Kマイクロ秒（ミリ秒）単位 */
    uint16_t capability;        /* Capability情報 */
    uint8_t SSID_len;
    uint8_t SSID[32];
    struct {
        uint32_t count;         /* このセットのrates数 */
        uint8_t rates[16];      /* 基本は500kbps単位でハイビットが設定されたレート */
    } rateset;                  /* サポートしているレート */
    uint16_t chanspec;          /* bssのchanspec */
    uint16_t atim_window;       /* Kマイクロ秒単位 */
    uint8_t dtim_period;        /* DTIM期間 */
    uint16_t RSSI;              /* 受信信号強度指標 (dBm単位) */
    int8_t phy_noise;               /* noise (in dBm) */

    uint8_t n_cap;              /* BSS は 802.11N 対応 */
    /* 802.11N BSS Capabilities (based on HT_CAP_*): */
    uint32_t nbss_cap;
    uint8_t ctl_ch;             /* 802.11N BSS 制御チャネル番号 */
    uint32_t reserved32[1];     /* Reserved for expansion of BSS properties */
    uint8_t flags;              /* フラグ */
    uint8_t reserved[3];        /* Reserved for expansion of BSS properties */
#define BRCMF_MCSSET_LEN        16
    uint8_t basic_mcs[BRCMF_MCSSET_LEN];    /* 802.11N BSS required MCS set */

    uint16_t ie_offset;         /* IEが始まる位置のこの構造体の先頭からのオフセット start, from beginning */
    uint32_t ie_length;         /* IE（情報要素）のバイト長 */
    uint16_t SNR;               /* フレーム受信中の平均SNR */
    /* ここに新しいいフィールドを追加する */
    /* ここから可変長のIEが続く */
};

struct brcmf_escan_result_le {
    uint32_t buflen;
    uint32_t version;
    uint16_t sync_id;
    uint16_t bss_count;
    struct brcmf_bss_info_le bss_info_le;
};

struct wpa_driver_xv6_data {
    void *ctx;
    struct bcm4343 *netdev;
    size_t ssid_len;
    uint8_t ssid[32];
    int country_set;
};

static int is_valid_country_code(const char *alpha2)
{
    assert(alpha2 != 0);
    return alpha2[0] == 'J' && alpha2[1] == 'P' && alpha2[2] == '\0';
}

#if 0
static const char countries[][3] =
{
    "AD","AE","AF","AI","AL","AM","AN","AR","AS","AT","AU","AW","AZ",
    "BA","BB","BD","BE","BF","BG","BH","BL","BM","BN","BO","BR","BS",
    "BT","BY","BZ","CA","CF","CH","CI","CL","CN","CO","CR","CU","CX",
    "CY","CZ","DE","DK","DM","DO","DZ","EC","EE","EG","ES","ET","FI",
    "FM","FR","GB","GD","GE","GF","GH","GL","GP","GR","GT","GU","GY",
    "HK","HN","HR","HT","HU","ID","IE","IL","IN","IR","IS","IT","JM",
    "JO","JP","KE","KH","KN","KP","KR","KW","KY","KZ","LB","LC","LI",
    "LK","LS","LT","LU","LV","MA","MC","MD","ME","MF","MH","MK","MN",
    "MO","MP","MQ","MR","MT","MU","MV","MW","MX","MY","NG","NI","NL",
    "NO","NP","NZ","OM","PA","PE","PF","PG","PH","PK","PL","PM","PR",
    "PT","PW","PY","QA","RE","RO","RS","RU","RW","SA","SE","SG","SI",
    "SK","SN","SR","SV","SY","TC","TD","TG","TH","TN","TR","TT","TW",
    "TZ","UA","UG","US","UY","UZ","VC","VE","VI","VN","VU","WF","WS",
    "YE","YT","ZA","ZW"
};

static int is_valid_country_code(const char *alpha2)
{
    assert(alpha2 != 0);

    for (unsigned i = 0; i < sizeof countries / sizeof countries[0]; i++)
    {
        if (   countries[i][0] == alpha2[0]
            && countries[i][1] == alpha2[1])
        {
            return 1;
        }
    }

    return 0;
}
#endif

static int wpa_driver_xv6_get_bssid(void *priv, uint8_t *bssid)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *)priv;
    assert(drv != 0);

    assert(drv->netdev != 0);
    assert(bssid);
    bssid = bcm4343_get_bssid(drv->netdev);

    return 0;
}

static int wpa_driver_xv6_get_ssid(void *priv, uint8_t *ssid)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *)priv;
    assert(drv != 0);

    assert(ssid != 0);
    if (drv->ssid_len > 0) {
        os_memcpy(ssid, drv->ssid, drv->ssid_len);
    }

    return drv->ssid_len;
}

static int wpa_driver_xv6_set_key(void *priv,
    struct wpa_driver_set_key_params *params)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);
    assert(drv->netdev != 0);

    enum wpa_alg alg = params->alg;
    const uint8_t *addr = params->addr;
    int key_idx = params->key_idx;
    int set_tx = params->set_tx;
    const uint8_t *seq = params->seq;
    size_t seq_len = params->seq_len;
    const uint8_t *key = params->key;
    size_t key_len = params->key_len;

    if (alg == WPA_ALG_NONE) {   // TODO: clear key
        return 0;
    }

    uint8_t key_tkip[32];
    if (alg == WPA_ALG_TKIP && key_len == 32) {
        // swap MIC keys, see set_key comment in driver.h
        os_memcpy(key_tkip, key, 16);
        os_memcpy(key_tkip+16, key+24, 8);
        os_memcpy(key_tkip+24, key+16, 8);

        key = key_tkip;
    }

    char number[2];
    cstring_t address;
    if (cstring_init(&address, 0) < 0) {
        wpa_printf(MSG_ERROR, "no memory for address\n");
        return -1;
    }
    for (int i = 0; i < ETH_ALEN; i++) {
        snprintf(number, 2, "%02X", addr[i]);
        if (cstring_append(&address, number) < 0) {
            wpa_printf(MSG_ERROR, "failed to append addr[%d]: %d\n", i, addr[i]);
            return -1;
        }
    }

    assert(alg == WPA_ALG_TKIP || alg == WPA_ALG_CCMP);
    struct cstring keys;
    if (cstring_init(&keys, 0) < 0) {
        wpa_printf(MSG_ERROR, "no memory for key\n");
        return -1;
    }
    if (cstring_append(&keys, alg == WPA_ALG_TKIP ? "tkip:" : "ccmp:") < 0) {
        wpa_printf(MSG_ERROR, "failed to append key head\n");
        return -1;
    }

    assert(key_len > 0);
    for (unsigned i = 0; i < key_len; i++) {
        snprintf(number, 2, "%02X", key[i]);
        if (cstring_append(&keys, number) < 0) {
            wpa_printf(MSG_ERROR, "failed to append key[%d]: %d\n", i, key[i]);
            return -1;
        }
    }

    if (cstring_append(&keys, "@") < 0) {
        wpa_printf(MSG_ERROR, "failed to append @\n");
        return -1;
    }

    assert(seq_len > 1);
    for (int i = seq_len-1; i >= 0; i--) {
        snprintf(number, 2, "%02X", seq[i]);
        if (cstring_append(&keys, number) < 0) {
            wpa_printf(MSG_ERROR, "failed to append seq[%d]: %d\n", i, seq[i]);
            return -1;
        }
    }

    char command[7];
    if (set_tx) {
        assert(key_idx == 0);
        safestrcpy(command, "txkey", 6);
    } else {
        assert(key_idx <= 3);
        snprintf(command, 7, "rxkey%u", key_idx);
    }

    if (!bcm4343_control(drv->netdev, "%s %s %s", (const char *)command,
                   (const char *) address.data, (const char *) keys.data)) {
        wpa_printf(MSG_ERROR, "failed bcm4343_control\n");
        return -1;
    }

    return 0;
}

static void wpa_driver_xv6_event_handler (
    ether_event_type_t type, const ether_event_params_t *params, void *context)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) context;
    assert(drv != 0);

    union wpa_event_data data;
    memset (&data, 0, sizeof data);

    switch (type) {
    case ether_event_link:        // ignore
        break;

    case ether_event_disassoc:
        drv->ssid_len = 0;
        wpa_supplicant_event(drv->ctx, EVENT_DISASSOC, 0);
        break;

    case ether_event_deauth:
        wpa_supplicant_event (drv->ctx, EVENT_DEAUTH, 0);
        break;

    case ether_event_mic_error:
        assert(params != 0);
        data.michael_mic_failure.unicast = !params->mic_error.group;
        data.michael_mic_failure.src = params->mic_error.addr;
        wpa_supplicant_event (drv->ctx, EVENT_MICHAEL_MIC_FAILURE, &data);
        break;

    default:
        wpa_printf (MSG_DEBUG, "Unhandled event %u", type);
        break;
    }
}

static void *wpa_driver_xv6_init(void *ctx, const char *ifname)
{
    struct net_device *netdev = net_device_by_index(NET_DEVICE_TYPE_WLAN);
    if (netdev == 0) {
        return 0;
    }

    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) os_zalloc (sizeof *drv);
    if (drv == 0) {
        return 0;
    }

    drv->ctx = ctx;
    drv->netdev = (struct bcm4343 *) netdev;    // netdev can only be of this type
    drv->country_set = 0;

    bcm4343_register_event_handler((struct bcm4343 *)netdev, wpa_driver_xv6_event_handler, drv);

    return drv;
}

static void wpa_driver_xv6_deinit(void *priv)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);

    bcm4343_register_event_handler(drv->netdev, 0, 0);

    os_free(drv);
}

#define SCAN_DURATION_SECS    3

static void wpa_driver_xv6_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) eloop_ctx;
    assert(drv != 0);

    assert(drv->netdev != 0);
    bcm4343_control(drv->netdev, "escan 0");    // stop scan

    wpa_supplicant_event(timeout_ctx, EVENT_SCAN_RESULTS, 0);
}

static int wpa_driver_xv6_scan2(void *priv,
    struct wpa_driver_scan_params *params)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);

    assert(params != 0);

    // TODO: allow scan params
    //assert(params->num_ssids == 0);
    assert(params->extra_ies == 0);
    assert(params->extra_ies_len == 0);
    assert(params->freqs == 0);

    assert(drv->netdev != 0);
    // increase scan duration here to be sure, scan is not started again
    if (!bcm4343_control(drv->netdev, "escan %u", SCAN_DURATION_SECS+2)) {
        return -1;
    }

    eloop_cancel_timeout(wpa_driver_xv6_scan_timeout, drv, drv->ctx);
    eloop_register_timeout(SCAN_DURATION_SECS, 0,
        wpa_driver_xv6_scan_timeout, drv, drv->ctx);

    return 0;
}

static int chanspec2freq(uint16_t chanspec)
{
    uint8_t chan = chanspec & 0xFF;

    if (1 <= chan && chan <= 14) {
        static const int low_freqs[] = {
            2412, 2417, 2422, 2427, 2432, 2437, 2442,
            2447, 2452, 2457, 2462, 2467, 2472, 2484
        };

        return low_freqs[chan-1];
    }

    if (32 <= chan && chan <= 173) {
        return 5160 + (chan-32) * 5;
    }

    return -1;
}

#define MAX_SCAN_RESULTS    128

static struct wpa_scan_results *wpa_driver_xv6_get_scan_results2(void *priv)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);

    struct wpa_scan_res **res_vector =
        (struct wpa_scan_res **) os_zalloc (MAX_SCAN_RESULTS * sizeof (struct wpa_scan_res *));
    if (res_vector == 0) {
        return 0;
    }

    struct wpa_scan_results *results = (struct wpa_scan_results *) os_zalloc (sizeof (struct wpa_scan_results));
    if (results == 0) {
        os_free(res_vector);
        return 0;
    }
    results->res = res_vector;
    results->num = 0;

    unsigned len;
    uint8_t buf[FRAME_BUFFER_SIZE];
    assert(drv->netdev != 0);
    while (bcm4343_recv_scan_result(drv->netdev, buf, &len)) {
        // remove remaining scan messages, if vector is full
        if (results->num == MAX_SCAN_RESULTS) {
            continue;
        }

        // TODO: validate escan result data
        struct brcmf_escan_result_le *scan_res = (struct brcmf_escan_result_le *) buf;

        struct brcmf_bss_info_le *bss = &scan_res->bss_info_le;
        for (unsigned i = 0; i < scan_res->bss_count; i++) {
            assert(bss->version == BRCMF_BSS_INFO_VERSION);

            int freq = chanspec2freq (bss->chanspec);
            if (freq <= 0) {
                continue;
            }

            struct wpa_scan_res *res =
                (struct wpa_scan_res *) os_zalloc (sizeof (struct wpa_scan_res) + bss->ie_length);
            if (res == 0) {
                break;
            }

            os_memset(res, 0, sizeof *res);

            res->flags = WPA_SCAN_LEVEL_DBM | WPA_SCAN_QUAL_INVALID;
            os_memcpy(res->bssid, bss->BSSID, ETH_ALEN);
            res->freq = freq;
            res->beacon_int = bss->beacon_period;
            res->caps = bss->capability;
            res->noise = bss->phy_noise;
            res->level = bss->RSSI;
            // TODO: set res->tsf
            // TODO: set res->age

            // append IEs
            res->ie_len = bss->ie_length;
            os_memcpy((uint8_t *) res + sizeof *res, (uint8_t *) bss + bss->ie_offset,
                   bss->ie_length);

            *res_vector++ = res;
            results->num++;

            bss = (struct brcmf_bss_info_le *) ((uint8_t *) bss + bss->length);
        }
    }

    return results;
}

static int wpa_driver_xv6_disassociate(void *priv, const uint8_t *own_addr,
                       const uint8_t *addr, uint16_t reason)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);

    assert(drv->netdev != 0);
    if (!bcm4343_control(drv->netdev, "disassoc %d", (int) reason)) {
        return -1;
    }

    return 0;
}

static int wpa_driver_xv6_associate(void *priv, struct wpa_driver_associate_params *params)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);
    assert(params != 0);

    struct cstring BSSID;
    if (cstring_init(&BSSID, "FFFFFFFFFFFF") < 0) {
        wpa_printf(MSG_ERROR, "failed init BSSID\n");
        return -1;
    }
    if (params->bssid != 0) {
        cstring_truncate(&BSSID);
        for (unsigned i = 0; i < ETH_ALEN; i++) {
            char number[2];
            snprintf(number, 2, "%02X", (unsigned) params->bssid[i]);
            if (cstring_append(&BSSID, number) < 0 ) {
                wpa_printf(MSG_ERROR, "failed append BSSID\n");
                return -1;
            }
        }
    }

    char ssid[32+1];
    assert(params->ssid != 0);
    assert(params->ssid_len < sizeof ssid);
    os_memcpy(ssid, params->ssid, params->ssid_len);
    ssid[params->ssid_len] = '\0';

    struct cstring SSID;
    if (cstring_init(&SSID, ssid) < 0) {
        wpa_printf(MSG_ERROR, "failed init SSID\n");
        return -1;
    }
    if (cstring_replace(&SSID, " ", "\\x20") < 0) {
        wpa_printf(MSG_ERROR, "failed replace SSID\n");
        return -1;
    }

    if (!(params->auth_alg & WPA_AUTH_ALG_OPEN)) {
        wpa_printf(MSG_ERROR, "Auth algorithm not supported (0x%X)\n", params->auth_alg);
        return -1;
    }

    int chan = 0;        // TODO: set channel from params->freq

    struct cstring Auth;
    if (cstring_init(&Auth, "off") < 0) {
        wpa_printf(MSG_ERROR, "failed init Auth\n");
        return -1;
    }
    if (params->wpa_ie != 0 && params->wpa_ie_len > 0) {

        cstring_truncate(&Auth);
        assert(params->wpa_ie != 0);
        for (unsigned i = 0; i < params->wpa_ie_len; i++) {
            char number[2];
            snprintf(number, 2, "%02X", (unsigned) params->wpa_ie[i]);
            if (cstring_append(&Auth, number) < 0) {
                wpa_printf(MSG_ERROR, "failed append BSSID\n");
                return -1;
            }
        }
    }

    if (!drv->country_set) {
        wpa_printf(MSG_ERROR, "Country code not set\n");
        return -1;
    }

    assert(drv->netdev != 0);
    if (!bcm4343_control(drv->netdev, "join %s %s %u %s",
        (const char *)SSID.data, (const char *) BSSID.data,
        chan, (const char *) Auth.data)) {
        return -1;
    }

    os_memcpy(drv->ssid, params->ssid, params->ssid_len);
    drv->ssid_len = params->ssid_len;

    wpa_supplicant_event(drv->ctx, EVENT_ASSOC, 0);

    return 0;
}

static int wpa_driver_xv6_set_country(void *priv, const char *alpha2)
{
    struct wpa_driver_xv6_data *drv = (struct wpa_driver_xv6_data *) priv;
    assert(drv != 0);

    char country[3];
    assert(alpha2 != 0);
    country[0] = alpha2[0];
    country[1] = alpha2[1];
    country[2] = '\0';

    if (!is_valid_country_code (alpha2)) {
        wpa_printf(MSG_ERROR, "Invalid country code: '%s'\n", country);
        return -1;
    }

    wpa_printf(MSG_INFO, "Setting country code to '%s'\n", country);

    assert(drv->netdev != 0);
    if (!bcm4343_control(drv->netdev, "country %s", country)) {
        return -1;
    }

    drv->country_set = 1;

    return 0;
}

const struct wpa_driver_ops wpa_driver_xv6_ops =
{
    .name = "xv6",
    .desc = "xv6 WLAN driver",
    .get_bssid = wpa_driver_xv6_get_bssid,
    .get_ssid = wpa_driver_xv6_get_ssid,
    .set_key = wpa_driver_xv6_set_key,
    .init = wpa_driver_xv6_init,
    .deinit = wpa_driver_xv6_deinit,
    .associate = wpa_driver_xv6_associate,
    .get_scan_results2 = wpa_driver_xv6_get_scan_results2,
    .set_country = wpa_driver_xv6_set_country,
    .scan2 = wpa_driver_xv6_scan2,
    .sta_disassoc = wpa_driver_xv6_disassociate,
};
