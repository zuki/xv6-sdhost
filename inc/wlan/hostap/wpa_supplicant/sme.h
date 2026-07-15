/*
 * wpa_supplicant - SME
 * Copyright (c) 2009-2010, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WPA_SUPPLICANT_SME_H
#define INC_WPA_SUPPLICANT_SME_H

#ifdef CONFIG_SME

void sme_authenticate(struct wpa_supplicant *wpa_s,
		      struct wpa_bss *bss, struct wpa_ssid *ssid);
void sme_associate(struct wpa_supplicant *wpa_s, enum wpas_mode mode,
		   const uint8_t *bssid, uint16_t auth_type);
void sme_event_auth(struct wpa_supplicant *wpa_s, union wpa_event_data *data);
int sme_update_ft_ies(struct wpa_supplicant *wpa_s, const uint8_t *md,
		      const uint8_t *ies, size_t ies_len);
void sme_event_assoc_reject(struct wpa_supplicant *wpa_s,
			    union wpa_event_data *data,
			    const uint8_t **link_bssids);
void sme_event_auth_timed_out(struct wpa_supplicant *wpa_s,
			      union wpa_event_data *data);
void sme_event_assoc_timed_out(struct wpa_supplicant *wpa_s,
			       union wpa_event_data *data);
void sme_event_disassoc(struct wpa_supplicant *wpa_s,
			struct disassoc_info *info);
void sme_event_unprot_disconnect(struct wpa_supplicant *wpa_s, const uint8_t *sa,
				 const uint8_t *da, uint16_t reason_code);
void sme_event_ch_switch(struct wpa_supplicant *wpa_s);
void sme_sa_query_rx(struct wpa_supplicant *wpa_s, const uint8_t *da, const uint8_t *sa,
		     const uint8_t *data, size_t len);
void sme_state_changed(struct wpa_supplicant *wpa_s);
void sme_clear_on_disassoc(struct wpa_supplicant *wpa_s);
void sme_deinit(struct wpa_supplicant *wpa_s);

int sme_proc_obss_scan(struct wpa_supplicant *wpa_s);
void sme_sched_obss_scan(struct wpa_supplicant *wpa_s, int enable);
void sme_external_auth_trigger(struct wpa_supplicant *wpa_s,
			       union wpa_event_data *data);
void sme_external_auth_mgmt_rx(struct wpa_supplicant *wpa_s,
			       const uint8_t *auth_frame, size_t len);

#else /* CONFIG_SME */

static inline void sme_authenticate(struct wpa_supplicant *wpa_s,
				    struct wpa_bss *bss,
				    struct wpa_ssid *ssid)
{
}

static inline void sme_event_auth(struct wpa_supplicant *wpa_s,
				  union wpa_event_data *data)
{
}

static inline int sme_update_ft_ies(struct wpa_supplicant *wpa_s, const uint8_t *md,
				    const uint8_t *ies, size_t ies_len)
{
	return -1;
}


static inline void sme_event_assoc_reject(struct wpa_supplicant *wpa_s,
					  union wpa_event_data *data,
					  const uint8_t **link_bssids)
{
}

static inline void sme_event_auth_timed_out(struct wpa_supplicant *wpa_s,
					    union wpa_event_data *data)
{
}

static inline void sme_event_assoc_timed_out(struct wpa_supplicant *wpa_s,
					     union wpa_event_data *data)
{
}

static inline void sme_event_disassoc(struct wpa_supplicant *wpa_s,
				      struct disassoc_info *info)
{
}

static inline void sme_event_unprot_disconnect(struct wpa_supplicant *wpa_s,
					       const uint8_t *sa, const uint8_t *da,
					       uint16_t reason_code)
{
}

static inline void sme_event_ch_switch(struct wpa_supplicant *wpa_s)
{
}

static inline void sme_state_changed(struct wpa_supplicant *wpa_s)
{
}

static inline void sme_clear_on_disassoc(struct wpa_supplicant *wpa_s)
{
}

static inline void sme_deinit(struct wpa_supplicant *wpa_s)
{
}

static inline int sme_proc_obss_scan(struct wpa_supplicant *wpa_s)
{
	return 0;
}

static inline void sme_sched_obss_scan(struct wpa_supplicant *wpa_s,
				       int enable)
{
}

static inline void sme_external_auth_trigger(struct wpa_supplicant *wpa_s,
					     union wpa_event_data *data)
{
}

static inline void sme_external_auth_mgmt_rx(struct wpa_supplicant *wpa_s,
					     const uint8_t *auth_frame, size_t len)
{
}

#endif /* CONFIG_SME */

#endif /* SME_H */
