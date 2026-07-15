/*
 * WPA Supplicant / shared MSCHAPV2 helper functions / RFC 2433 / RFC 2759
 * Copyright (c) 2004-2009, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INC_WLAN_CRYPTO_MS_FUNCS_H
#define INC_WLAN_CRYPTO_MS_FUNCS_H

#include <types.h>

int generate_nt_response(const uint8_t *auth_challenge, const uint8_t *peer_challenge,
			 const uint8_t *username, size_t username_len,
			 const uint8_t *password, size_t password_len,
			 uint8_t *response);
int generate_nt_response_pwhash(const uint8_t *auth_challenge,
				const uint8_t *peer_challenge,
				const uint8_t *username, size_t username_len,
				const uint8_t *password_hash,
				uint8_t *response);
int generate_authenticator_response(const uint8_t *password, size_t password_len,
				    const uint8_t *peer_challenge,
				    const uint8_t *auth_challenge,
				    const uint8_t *username, size_t username_len,
				    const uint8_t *nt_response, uint8_t *response);
int generate_authenticator_response_pwhash(
	const uint8_t *password_hash,
	const uint8_t *peer_challenge, const uint8_t *auth_challenge,
	const uint8_t *username, size_t username_len,
	const uint8_t *nt_response, uint8_t *response);
int nt_challenge_response(const uint8_t *challenge, const uint8_t *password,
			  size_t password_len, uint8_t *response);

int challenge_response(const uint8_t *challenge, const uint8_t *password_hash,
		       uint8_t *response);
int challenge_hash(const uint8_t *peer_challenge, const uint8_t *auth_challenge,
		   const uint8_t *username, size_t username_len, uint8_t *challenge);
int nt_password_hash(const uint8_t *password, size_t password_len,
		     uint8_t *password_hash);
int hash_nt_password_hash(const uint8_t *password_hash, uint8_t *password_hash_hash);
int get_master_key(const uint8_t *password_hash_hash, const uint8_t *nt_response,
		   uint8_t *master_key);
int get_asymetric_start_key(const uint8_t *master_key, uint8_t *session_key,
			    size_t session_key_len, int is_send,
			    int is_server);
int encrypt_pw_block_with_password_hash(
	const uint8_t *password, size_t password_len,
	const uint8_t *password_hash, uint8_t *pw_block);
int new_password_encrypted_with_old_nt_password_hash(
	const uint8_t *new_password, size_t new_password_len,
	const uint8_t *old_password, size_t old_password_len,
	uint8_t *encrypted_pw_block);
int nt_password_hash_encrypted_with_block(const uint8_t *password_hash,
					  const uint8_t *block, uint8_t *cypher);
int old_nt_password_hash_encrypted_with_new_nt_password_hash(
	const uint8_t *new_password, size_t new_password_len,
	const uint8_t *old_password, size_t old_password_len,
	uint8_t *encrypted_password_hash);

#endif /* MS_FUNCS_H */
