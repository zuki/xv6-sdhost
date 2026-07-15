/*
 * wpa_supplicant - TWT
 * Copyright (c) 2003-2016, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <utils/includes.h>

#include <utils/common.h>
#include <wpa_supplicant_i.h>
#include <driver_i.h>


#ifdef CONFIG_TESTING_OPTIONS

/**
 * wpas_twt_send_setup - Send TWT Setup frame (Request) to our AP
 * @wpa_s: Pointer to wpa_supplicant
 * @dtok: Dialog token
 * @exponent: Wake-interval exponent
 * @mantissa: Wake-interval mantissa
 * @min_twt: Minimum TWT wake duration in units of 256 usec
 * @setup_cmd: 0 == request, 1 == suggest, etc.  Table 9-297
 * Returns: 0 in case of success, negative error code otherwise
 *
 */
int wpas_twt_send_setup(struct wpa_supplicant *wpa_s, uint8_t dtok, int exponent,
			int mantissa, uint8_t min_twt, int setup_cmd, uint64_t twt,
			bool requestor, bool trigger, bool implicit,
			bool flow_type, uint8_t flow_id, bool protection,
			uint8_t twt_channel, uint8_t control)
{
	struct wpabuf *buf;
	uint16_t req_type = 0;
	int ret = 0;

	if (wpa_s->wpa_state != WPA_COMPLETED || !wpa_s->current_ssid) {
		wpa_printf(MSG_DEBUG,
			   "TWT: No connection - cannot send TWT Setup frame");
		return -ENOTCONN;
	}

	/* 3 = Action category + Action code + Dialog token */
	/* 17 = TWT element */
	buf = wpabuf_alloc(3 + 17);
	if (!buf) {
		wpa_printf(MSG_DEBUG,
			   "TWT: Failed to allocate TWT Setup frame (Request)");
		return -ENOMEM;
	}

	wpa_printf(MSG_DEBUG,
		   "TWT: Setup request, dtok: %d  exponent: %d  mantissa: %d  min-twt: %d",
		   dtok, exponent, mantissa, min_twt);

	wpabuf_put_uint8_t(buf, WLAN_ACTION_S1G);
	wpabuf_put_uint8_t(buf, S1G_ACT_TWT_SETUP);
	wpabuf_put_uint8_t(buf, dtok);

	wpabuf_put_uint8_t(buf, WLAN_EID_TWT);
	wpabuf_put_uint8_t(buf, 15); /* len */

	wpabuf_put_uint8_t(buf, control);

	if (requestor)
		req_type |= BIT(0); /* This STA is a TWT Requesting STA */
	/* TWT Setup Command field */
	req_type |= (setup_cmd & 0x7) << 1;
	if (trigger)
		req_type |= BIT(4); /* TWT SP includes trigger frames */
	if (implicit)
		req_type |= BIT(5); /* Implicit TWT */
	if (flow_type)
		req_type |= BIT(6); /* Flow Type: Unannounced TWT */
	req_type |= (flow_id & 0x7) << 7;
	req_type |= (exponent & 0x1f) << 10; /* TWT Wake Interval Exponent */
	if (protection)
		req_type |= BIT(15);
	wpabuf_put_le16(buf, req_type);
	wpabuf_put_le64(buf, twt);
	wpabuf_put_uint8_t(buf, min_twt); /* Nominal Minimum TWT Wake Duration */
	wpabuf_put_le16(buf, mantissa); /* TWT Wake Interval Mantissa */
	wpabuf_put_uint8_t(buf, twt_channel); /* TWT Channel */

	if (wpa_drv_send_action(wpa_s, wpa_s->assoc_freq, 0, wpa_s->bssid,
				wpa_s->own_addr, wpa_s->bssid,
				wpabuf_head(buf), wpabuf_len(buf), 0) < 0) {
		wpa_printf(MSG_DEBUG, "TWT: Failed to send TWT Setup Request");
		ret = -ECANCELED;
	}

	wpabuf_free(buf);
	return ret;
}


/**
 * wpas_twt_send_teardown - Send TWT teardown request to our AP
 * @wpa_s: Pointer to wpa_supplicant
 * @flags: The byte that goes inside the TWT Teardown element
 * Returns: 0 in case of success, negative error code otherwise
 *
 */
int wpas_twt_send_teardown(struct wpa_supplicant *wpa_s, uint8_t flags)
{
	struct wpabuf *buf;
	int ret = 0;

	if (wpa_s->wpa_state != WPA_COMPLETED || !wpa_s->current_ssid) {
		wpa_printf(MSG_DEBUG,
			   "TWT: No connection - cannot send TWT Teardown frame");
		return -ENOTCONN;
	}

	/* 3 = Action category + Action code + flags */
	buf = wpabuf_alloc(3);
	if (!buf) {
		wpa_printf(MSG_DEBUG,
			   "TWT: Failed to allocate TWT Teardown frame");
		return -ENOMEM;
	}

	wpa_printf(MSG_DEBUG, "TWT: Teardown request, flags: 0x%x", flags);

	wpabuf_put_uint8_t(buf, WLAN_ACTION_S1G);
	wpabuf_put_uint8_t(buf, S1G_ACT_TWT_TEARDOWN);
	wpabuf_put_uint8_t(buf, flags);

	if (wpa_drv_send_action(wpa_s, wpa_s->assoc_freq, 0, wpa_s->bssid,
				wpa_s->own_addr, wpa_s->bssid,
				wpabuf_head(buf), wpabuf_len(buf), 0) < 0) {
		wpa_printf(MSG_DEBUG, "TWT: Failed to send TWT Teardown frame");
		ret = -ECANCELED;
	}

	wpabuf_free(buf);
	return ret;
}

#endif /* CONFIG_TESTING_OPTIONS */
