#pragma once

#include "Crypto.h"
#include "../message/Messages.h"
#include "monocypher/monocypher.h"
#include "esp_random.h"

namespace Crypto
{
  // <========================>
  //   Nonce Functions
  // <========================>
  void generateNonce(uint8_t* outNonce)
  {
    // Fill nonce with secure random bytes
    for (size_t i = 0; i < NONCE_SIZE; i += 4)
    {
      uint32_t r = esp_random();
      memcpy(outNonce + i, &r, sizeof(r));
    }
  }

  // <========================>
  //   Key-Pair Functions
  // <========================>
  void generateKeyPair(uint8_t* outPrivKey, uint8_t* outPubKey)
  {
    // Fill private key with secure random bytes
    for (size_t i = 0; i < KEY_SIZE; i += 4)
    {
      uint32_t r = esp_random();
      memcpy(outPrivKey + i, &r, sizeof(r));
    }

    // Compute corresponding public key
    crypto_x25519_public_key(outPubKey, outPrivKey);
  }

  // <========================>
  //   Shared Secret Functions
  // <========================>
  bool computeSharedSecret(const uint8_t* myPrivKey, const uint8_t* peerPubKey, uint8_t* outSharedSecret)
  {
    if (!peerPubKey) return false;

    crypto_x25519(outSharedSecret, myPrivKey, peerPubKey);
    return true;
  }

  // <========================>
  //   Session Key Functions
  // <========================>
  void deriveSessionKey(uint64_t myId, uint64_t peerId, const uint8_t* sharedSecret, const uint8_t* myNonce, const uint8_t* peerNonce, const uint8_t* psk, uint8_t* outKey)
  {
    uint8_t buffer[ID_SIZE + ID_SIZE + NONCE_SIZE + NONCE_SIZE + PSK_SIZE];
    size_t offset = 0;

    const uint8_t* firstNonce;
    const uint8_t* secondNonce;
    uint64_t firstId;
    uint64_t secondId;

    // Deterministic Ordering
    if (myId < peerId)
    {
      firstId = myId;
      secondId = peerId;

      firstNonce = myNonce;
      secondNonce = peerNonce;
    }
    else
    {
      firstId = peerId;
      secondId = myId;

      firstNonce = peerNonce;
      secondNonce = myNonce;
    }

    // Pack IDs
    for (int i = 0; i < ID_SIZE; i++) {
      buffer[offset + i] = (firstId >> (8 * (7 - i))) & 0xFF;
    }
    offset += 8;

    // Serialize secondId (big-endian)
    for (int i = 0; i < ID_SIZE; i++) {
      buffer[offset + i] = (secondId >> (8 * (7 - i))) & 0xFF;
    }
    offset += 8;

    // Pack nonces
    memcpy(buffer + offset, firstNonce, NONCE_SIZE); offset += NONCE_SIZE;
    memcpy(buffer + offset, secondNonce, NONCE_SIZE); offset += NONCE_SIZE;

    // Optional PSK
    if (psk != NULL)
    {
      memcpy(buffer + offset, psk, PSK_SIZE);
      offset += PSK_SIZE;
    }

    // Derive session key
    crypto_blake2b_keyed(
      outKey, KEY_SIZE,
      sharedSecret, KEY_SIZE,
      buffer, offset
    );
  }

  void blake2bMAC(uint8_t* out, const uint8_t* key, const uint8_t* data, size_t dataLen)
  {
    crypto_blake2b_keyed(
      out, KEY_SIZE,
      key, KEY_SIZE,
      data, dataLen
    );
  }

  int verify32(const uint8_t a[32], const uint8_t b[32])
  {
    return crypto_verify32(a, b);
  }

  void encrypt(const uint8_t* key, const uint8_t* nonce, const uint8_t* plaintext, size_t len, uint8_t* outCiphertext, uint8_t* outTag)
  {
    // uint8_t nonce[COUNTER_NONCE_SIZE];
    // buildCounterNonce(counter, nonce);

    crypto_aead_ctx ctx;
    crypto_aead_init_ietf(&ctx, key, nonce);

    crypto_aead_write(
      &ctx, 
      outCiphertext, 
      outTag, 
      NULL, 0,  // AAD
      plaintext, 
      len
    );
  }

  bool decrypt(const uint8_t* key, const uint8_t* nonce, const uint8_t* ciphertext, size_t len, const uint8_t* tag, uint8_t* outPlaintext)
  {
    // uint8_t nonce[COUNTER_NONCE_SIZE];
    // buildCounterNonce(counter, nonce);

    crypto_aead_ctx ctx;
    crypto_aead_init_ietf(&ctx, key, nonce);

    int ok = crypto_aead_read(
      &ctx,
      outPlaintext,
      tag,
      NULL, 0, // AAD
      ciphertext,
      len
    );

    return ok == 0; // 0 = success
  }

  // Wipe secrets if they are no longer needed
  void wipe(void *secret, size_t secret_size)
  {
    crypto_wipe(secret, secret_size);
  }
}
