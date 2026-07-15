/*
 * AES-based functions
 *
 * - AES Key Wrap Algorithm (RFC3394)
 * - One-Key CBC MAC (OMAC1) hash with AES-128 and AES-256
 * - AES-128/192/256 CTR mode encryption
 * - AES-128 EAX mode encryption/decryption
 * - AES-128 CBC
 * - AES-GCM
 * - AES-CCM
 *
 * Copyright (c) 2003-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_AES_WRAP_H
#define INC_WLAN_CRYPTO_AES_WRAP_H

#include <types.h>

int aes_wrap(const uint8_t *kek, size_t kek_len, int n, const uint8_t *plain,
			  uint8_t *cipher);
int aes_unwrap(const uint8_t *kek, size_t kek_len, int n,
			    const uint8_t *cipher, uint8_t *plain);
int omac1_aes_vector(const uint8_t *key, size_t key_len,
				  size_t num_elem, const uint8_t *addr[],
				  const size_t *len, uint8_t *mac);
int omac1_aes_128_vector(const uint8_t *key, size_t num_elem,
				      const uint8_t *addr[], const size_t *len,
				      uint8_t *mac);
int omac1_aes_128(const uint8_t *key, const uint8_t *data, size_t data_len,
			       uint8_t *mac);
int omac1_aes_256(const uint8_t *key, const uint8_t *data, size_t data_len,
			       uint8_t *mac);
int aes_128_encrypt_block(const uint8_t *key, const uint8_t *in, uint8_t *out);
int aes_ctr_encrypt(const uint8_t *key, size_t key_len, const uint8_t *nonce,
				 uint8_t *data, size_t data_len);
int aes_128_ctr_encrypt(const uint8_t *key, const uint8_t *nonce,
				     uint8_t *data, size_t data_len);
int aes_128_eax_encrypt(const uint8_t *key,
				     const uint8_t *nonce, size_t nonce_len,
				     const uint8_t *hdr, size_t hdr_len,
				     uint8_t *data, size_t data_len, uint8_t *tag);
int aes_128_eax_decrypt(const uint8_t *key,
				     const uint8_t *nonce, size_t nonce_len,
				     const uint8_t *hdr, size_t hdr_len,
				     uint8_t *data, size_t data_len, const uint8_t *tag);
int aes_128_cbc_encrypt(const uint8_t *key, const uint8_t *iv, uint8_t *data,
				     size_t data_len);
int aes_128_cbc_decrypt(const uint8_t *key, const uint8_t *iv, uint8_t *data,
				     size_t data_len);
int aes_gcm_ae(const uint8_t *key, size_t key_len,
			    const uint8_t *iv, size_t iv_len,
			    const uint8_t *plain, size_t plain_len,
			    const uint8_t *aad, size_t aad_len,
			    uint8_t *crypt, uint8_t *tag);
int aes_gcm_ad(const uint8_t *key, size_t key_len,
			    const uint8_t *iv, size_t iv_len,
			    const uint8_t *crypt, size_t crypt_len,
			    const uint8_t *aad, size_t aad_len, const uint8_t *tag,
			    uint8_t *plain);
int aes_gmac(const uint8_t *key, size_t key_len,
			  const uint8_t *iv, size_t iv_len,
			  const uint8_t *aad, size_t aad_len, uint8_t *tag);
int aes_ccm_ae(const uint8_t *key, size_t key_len, const uint8_t *nonce,
			    size_t M, const uint8_t *plain, size_t plain_len,
			    const uint8_t *aad, size_t aad_len, uint8_t *crypt, uint8_t *auth);
int aes_ccm_ad(const uint8_t *key, size_t key_len, const uint8_t *nonce,
			    size_t M, const uint8_t *crypt, size_t crypt_len,
			    const uint8_t *aad, size_t aad_len, const uint8_t *auth,
			    uint8_t *plain);

#endif /* AES_WRAP_H */
