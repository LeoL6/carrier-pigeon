#pragma once
#include <stdint.h>

struct Session
{
  uint64_t myId;
  uint64_t peerId;

  uint8_t psk[12];

  uint8_t myNonce[16];
  uint8_t peerNonce[16];

  uint8_t myPrivKey[32];
  uint8_t myPubKey[32];
  uint8_t peerPubKey[32];

  uint8_t sharedSecret[32];
  uint8_t sessionKey[32];

  uint64_t txCounter;     // For outgoing messages
  uint64_t rxCounter;     // Last seen incoming counter
};

namespace SessionManager
{
  Session& get();
  void reset();
}