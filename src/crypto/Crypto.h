#pragma once

#include <stdint.h>
#include <stddef.h>

// --- Sizes ---
#define CRYPTO_NONCE_SIZE 16
#define CRYPTO_KEY_SIZE   32   // 256-bit key
#define CRYPTO_HASH_SIZE  32   // SHA-256 output

// --- API ---

// Fill buffer with secure random bytes
bool crypto_random_bytes(uint8_t* buf, size_t len);

// HMAC-SHA256
bool crypto_hmac_sha256(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t* out // 32 bytes
);

// HKDF-SHA256
bool crypto_hkdf_sha256(
    const uint8_t* ikm, size_t ikm_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* out_key, size_t out_len
);