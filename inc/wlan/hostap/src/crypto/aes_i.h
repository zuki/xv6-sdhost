/*
 * AES (Rijndael) cipher
 * Copyright (c) 2003-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_AES_I_H
#define INC_WLAN_CRYPTO_AES_I_H

#include <crypto/aes.h>

/* #define FULL_UNROLL */
#define AES_SMALL_TABLES

extern const uint32_t Te0[256];
extern const uint32_t Te1[256];
extern const uint32_t Te2[256];
extern const uint32_t Te3[256];
extern const uint32_t Te4[256];
extern const uint32_t Td0[256];
extern const uint32_t Td1[256];
extern const uint32_t Td2[256];
extern const uint32_t Td3[256];
extern const uint32_t Td4[256];
extern const uint32_t rcon[10];
extern const uint8_t Td4s[256];
extern const uint8_t rcons[10];

#ifndef AES_SMALL_TABLES

#define RCON(i) rcon[(i)]

#define TE0(i) Te0[((i) >> 24) & 0xff]
#define TE1(i) Te1[((i) >> 16) & 0xff]
#define TE2(i) Te2[((i) >> 8) & 0xff]
#define TE3(i) Te3[(i) & 0xff]
#define TE41(i) (Te4[((i) >> 24) & 0xff] & 0xff000000)
#define TE42(i) (Te4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE43(i) (Te4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE44(i) (Te4[(i) & 0xff] & 0x000000ff)
#define TE421(i) (Te4[((i) >> 16) & 0xff] & 0xff000000)
#define TE432(i) (Te4[((i) >> 8) & 0xff] & 0x00ff0000)
#define TE443(i) (Te4[(i) & 0xff] & 0x0000ff00)
#define TE414(i) (Te4[((i) >> 24) & 0xff] & 0x000000ff)
#define TE411(i) (Te4[((i) >> 24) & 0xff] & 0xff000000)
#define TE422(i) (Te4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE433(i) (Te4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE444(i) (Te4[(i) & 0xff] & 0x000000ff)
#define TE4(i) (Te4[(i)] & 0x000000ff)

#define TD0(i) Td0[((i) >> 24) & 0xff]
#define TD1(i) Td1[((i) >> 16) & 0xff]
#define TD2(i) Td2[((i) >> 8) & 0xff]
#define TD3(i) Td3[(i) & 0xff]
#define TD41(i) (Td4[((i) >> 24) & 0xff] & 0xff000000)
#define TD42(i) (Td4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TD43(i) (Td4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TD44(i) (Td4[(i) & 0xff] & 0x000000ff)
#define TD0_(i) Td0[(i) & 0xff]
#define TD1_(i) Td1[(i) & 0xff]
#define TD2_(i) Td2[(i) & 0xff]
#define TD3_(i) Td3[(i) & 0xff]

#else /* AES_SMALL_TABLES */

#define RCON(i) ((uint32_t) rcons[(i)] << 24)

static inline uint32_t rotr(uint32_t val, int bits)
{
	return (val >> bits) | (val << (32 - bits));
}

#define TE0(i) Te0[((i) >> 24) & 0xff]
#define TE1(i) rotr(Te0[((i) >> 16) & 0xff], 8)
#define TE2(i) rotr(Te0[((i) >> 8) & 0xff], 16)
#define TE3(i) rotr(Te0[(i) & 0xff], 24)
#define TE41(i) ((Te0[((i) >> 24) & 0xff] << 8) & 0xff000000)
#define TE42(i) (Te0[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE43(i) (Te0[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE44(i) ((Te0[(i) & 0xff] >> 8) & 0x000000ff)
#define TE421(i) ((Te0[((i) >> 16) & 0xff] << 8) & 0xff000000)
#define TE432(i) (Te0[((i) >> 8) & 0xff] & 0x00ff0000)
#define TE443(i) (Te0[(i) & 0xff] & 0x0000ff00)
#define TE414(i) ((Te0[((i) >> 24) & 0xff] >> 8) & 0x000000ff)
#define TE411(i) ((Te0[((i) >> 24) & 0xff] << 8) & 0xff000000)
#define TE422(i) (Te0[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE433(i) (Te0[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE444(i) ((Te0[(i) & 0xff] >> 8) & 0x000000ff)
#define TE4(i) ((Te0[(i)] >> 8) & 0x000000ff)

#define TD0(i) Td0[((i) >> 24) & 0xff]
#define TD1(i) rotr(Td0[((i) >> 16) & 0xff], 8)
#define TD2(i) rotr(Td0[((i) >> 8) & 0xff], 16)
#define TD3(i) rotr(Td0[(i) & 0xff], 24)
#define TD41(i) ((uint32_t) Td4s[((i) >> 24) & 0xff] << 24)
#define TD42(i) ((uint32_t) Td4s[((i) >> 16) & 0xff] << 16)
#define TD43(i) ((uint32_t) Td4s[((i) >> 8) & 0xff] << 8)
#define TD44(i) ((uint32_t) Td4s[(i) & 0xff])
#define TD0_(i) Td0[(i) & 0xff]
#define TD1_(i) rotr(Td0[(i) & 0xff], 8)
#define TD2_(i) rotr(Td0[(i) & 0xff], 16)
#define TD3_(i) rotr(Td0[(i) & 0xff], 24)

#endif /* AES_SMALL_TABLES */

#ifdef _MSC_VER
#define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
#define GETU32(p) SWAP(*((uint32_t *)(p)))
#define PUTU32(ct, st) { *((uint32_t *)(ct)) = SWAP((st)); }
#else
#define GETU32(pt) (((uint32_t)(pt)[0] << 24) ^ ((uint32_t)(pt)[1] << 16) ^ \
((uint32_t)(pt)[2] <<  8) ^ ((uint32_t)(pt)[3]))
#define PUTU32(ct, st) { \
(ct)[0] = (uint8_t)((st) >> 24); (ct)[1] = (uint8_t)((st) >> 16); \
(ct)[2] = (uint8_t)((st) >>  8); (ct)[3] = (uint8_t)(st); }
#endif

#define AES_PRIV_SIZE (4 * 4 * 15 + 4)
#define AES_PRIV_NR_POS (4 * 15)

int rijndaelKeySetupEnc(uint32_t rk[], const uint8_t cipherKey[], int keyBits);

#endif /* AES_I_H */
