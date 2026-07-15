/*
 * EAP-WSC definitions for Wi-Fi Protected Setup
 * Copyright (c) 2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_EAP_COMMON_EAP_WSC_COMMON_H
#define INC_WLAN_EAP_COMMON_EAP_WSC_COMMON_H

#include <types.h>

#define EAP_VENDOR_TYPE_WSC 1

#define WSC_FLAGS_MF 0x01
#define WSC_FLAGS_LF 0x02

#define WSC_ID_REGISTRAR "WFA-SimpleConfig-Registrar-1-0"
#define WSC_ID_REGISTRAR_LEN 30
#define WSC_ID_ENROLLEE "WFA-SimpleConfig-Enrollee-1-0"
#define WSC_ID_ENROLLEE_LEN 29

#define WSC_FRAGMENT_SIZE 1400


struct wpabuf * eap_wsc_build_frag_ack(uint8_t id, uint8_t code);

#endif /* EAP_WSC_COMMON_H */
