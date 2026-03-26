#pragma once

#include "../protocol/Packet.h"

namespace Crypto
{
  // <========================>
  //   Constants
  // <========================>
  constexpr size_t ID_SIZE            = 8;
  constexpr size_t COUNTER_SIZE       = 8;
  constexpr size_t PSK_SIZE           = 12;
  constexpr size_t COUNTER_NONCE_SIZE = 12; 
  constexpr size_t NONCE_SIZE         = 16;
  constexpr size_t TAG_SIZE           = 16;
  constexpr size_t KEY_SIZE           = 32;

  // <========================>
  //   Nonce Functions
  // <========================>
  void generateNonce(uint8_t* outNonce);
  void buildCounterNonce(uint64_t counter, uint8_t* nonce);

  // <========================>
  //   Key-Pair Functions
  // <========================>
  void generateKeyPair(uint8_t* outPrivKey, uint8_t* outPubKey);

  // <========================>
  //   Shared Secret Functions
  // <========================>
  bool computeSharedSecret(const uint8_t* myPrivKey, const uint8_t* peerPubKey, uint8_t* outSharedSecret);

  // <========================>
  //   Session Key Functions
  // <========================>
  void deriveSessionKey(uint64_t myId, uint64_t peerId, const uint8_t* sharedSecret, const uint8_t* myNonce, const uint8_t* peerNonce, const uint8_t* psk, uint8_t* outKey);

  // <========================>
  //   Verification Functions
  // <========================>
  void blake2bMAC(uint8_t* out, const uint8_t* key, const uint8_t* data, size_t dataLen);
  int verify32(const uint8_t a[32], const uint8_t b[32]);

  // <========================>
  //   Encryption Functions
  // <========================>
  void encrypt(const uint8_t* key, uint64_t counter, const uint8_t* plaintext, size_t len, uint8_t* outCiphertext, uint8_t* outTag);
  bool decrypt(const uint8_t* key, uint64_t counter, const uint8_t* ciphertext, size_t len, const uint8_t* tag, uint8_t* outPlaintext);
}
