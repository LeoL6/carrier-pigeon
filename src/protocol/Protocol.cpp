#include "Protocol.h"

#include "../crypto/Crypto.h"
#include "../radio/LoRa.h"

#include "Handshake.h"
#include "Session.h"

namespace Protocol
{
  Session& session = SessionManager::get();

  // <========================>
  //   Callback
  // <========================>
  static DataCallback onDataCb = nullptr;

  void setDataCallback(DataCallback cb)
  {
    onDataCb = cb;
  }

  static void onData(Packet::Packet& pkt)
  {
   if (!Handshake::isEstablished()) return;

    // Deserialize counter from big-endian
    uint64_t counter = 0;
    for (size_t i = 0; i < Crypto::COUNTER_SIZE; i++) {
      counter = (counter << 8) | pkt.payload[i];
    }

    // Replay protection
    if (counter <= session.rxCounter) {
      Serial.println("Replay detected");
      return;
    }

    // Compute lengths
    size_t cipherLen = pkt.length - Crypto::COUNTER_SIZE - Crypto::TAG_SIZE;
    const uint8_t* ciphertext = pkt.payload + Crypto::COUNTER_SIZE;
    const uint8_t* tag = pkt.payload + Crypto::COUNTER_SIZE + cipherLen;

    // Rebuild nonce from counter
    uint8_t nonce[Crypto::COUNTER_NONCE_SIZE];
    memset(nonce, 0, Crypto::COUNTER_NONCE_SIZE);
    for (int i = 0; i < Crypto::COUNTER_SIZE; i++) {
      nonce[Crypto::COUNTER_NONCE_SIZE - 1 - i] = (counter >> (8 * i)) & 0xFF;
    }

    // Decrypt into exact message length
    uint8_t plaintext[64];
    bool ok = Crypto::decrypt(
      session.sessionKey,
      counter,
      ciphertext,
      cipherLen,
      tag,
      plaintext
    );

    if (!ok) {
      Serial.println("Decrypt failed (tampered)");
      return;
    }

    session.rxCounter = counter; // Update last received counter

    // Null-terminate exactly at decrypted length
    if (cipherLen < sizeof(plaintext))
      plaintext[cipherLen] = '\0';

    if (onDataCb)
      onDataCb(plaintext, cipherLen);
  }

  // <========================>
  //   Packet Rx Function
  // <========================>
  void onReceive(Packet::Packet& pkt)
  {
    switch (pkt.type)
    {
      case Packet::HELLO:
        Serial.println("RECEIVED HELLO");
        Handshake::onHello(pkt);
        break;

      case Packet::HELLO_ACK:
        Serial.println("RECEIVED HELLO ACK");
        Handshake::onHelloAck(pkt);
        break;

      case Packet::AUTH:
        Serial.println("RECEIVED AUTH");
        Handshake::onAuth(pkt);
        break;

      case Packet::AUTH_ACK:
        Serial.println("RECEIVED AUTH ACK");
        Handshake::onAuthAck(pkt);
        break;

      case Packet::DATA:
        Serial.println("RECEIVED DATA");
        onData(pkt);
        break;

      default:
        // ignore unknown packet types
        break;
    }
  }

  void sendData(const uint8_t* data, size_t len)
  {
    if (len > 64) return;  // Limit message size

    uint8_t ciphertext[64];        // Max message size
    uint8_t tag[Crypto::TAG_SIZE]; // 16 bytes

    uint64_t counter = session.txCounter++;

    // Build nonce from counter (12 bytes)
    uint8_t nonce[Crypto::COUNTER_NONCE_SIZE];
    memset(nonce, 0, Crypto::COUNTER_NONCE_SIZE);
    for (int i = 0; i < Crypto::COUNTER_SIZE; i++) {
      nonce[Crypto::COUNTER_NONCE_SIZE - 1 - i] = (counter >> (8 * i)) & 0xFF;
    }

    // Encrypt using Monocypher
    Crypto::encrypt(
      session.sessionKey,
      counter,
      data,
      len,
      ciphertext,
      tag
    );

    // Build packet
    uint8_t payload[Crypto::COUNTER_SIZE + len + Crypto::TAG_SIZE];
    size_t offset = 0;

    // Serialize counter as big-endian
    for (int i = 0; i < Crypto::COUNTER_SIZE; i++) {
      payload[offset++] = (counter >> (8 * (Crypto::COUNTER_SIZE - 1 - i))) & 0xFF;
    }

    // Copy ciphertext and tag exactly
    memcpy(payload + offset, ciphertext, len); offset += len;
    memcpy(payload + offset, tag, Crypto::TAG_SIZE); offset += Crypto::TAG_SIZE;

    LoRa::enqueuePacket(Packet::DATA, payload, offset);
  }
}