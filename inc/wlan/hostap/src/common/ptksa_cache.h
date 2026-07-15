/*
 * RSN PTKSA cache interface
 *
 * Copyright (C) 2019 Intel Corporation
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_COMMON_PTKSA_CACHE_H
#define INC_WLAN_COMMON_PTKSA_CACHE_H

#include <common/wpa_common.h>
#include <common/defs.h>
#include <utils/list.h>

/**
 * struct ptksa_cache_entry - PTKSA cache entry
 */
struct ptksa_cache_entry {
	struct dl_list list;
	struct wpa_ptk ptk;
	os_time_t expiration;
	uint32_t cipher;
	uint8_t addr[ETH_ALEN];
	uint8_t own_addr[ETH_ALEN];
	void (*cb)(struct ptksa_cache_entry *e);
	void *ctx;
	uint32_t akmp;
};


struct ptksa_cache;

struct ptksa_cache * ptksa_cache_init(void);
void ptksa_cache_deinit(struct ptksa_cache *ptksa);
struct ptksa_cache_entry * ptksa_cache_get(struct ptksa_cache *ptksa,
					   const uint8_t *addr, uint32_t cipher);
int ptksa_cache_list(struct ptksa_cache *ptksa, char *buf, size_t len);
struct ptksa_cache_entry * ptksa_cache_add(struct ptksa_cache *ptksa,
					   const uint8_t *own_addr,
					   const uint8_t *addr, uint32_t cipher,
					   uint32_t life_time,
					   const struct wpa_ptk *ptk,
					   void (*cb)
					   (struct ptksa_cache_entry *e),
					   void *ctx, uint32_t akmp);
void ptksa_cache_flush(struct ptksa_cache *ptksa, const uint8_t *addr, uint32_t cipher);

#endif /* PTKSA_CACHE_H */
