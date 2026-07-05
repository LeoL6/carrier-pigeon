#include "Protocol.h"

#include "../compression/Compression.h"
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

  static void buildCounterNonce(uint8_t senderRole, uint64_t counter, uint8_t* nonce)
  {
    // Zero the first 4 bytes
    // memset(nonce, 0, Crypto::COUNTER_NONCE_SIZE - Crypto::COUNTER_SIZE);
    memset(nonce, 0, Crypto::COUNTER_NONCE_SIZE);

    nonce[0] = senderRole;

    // Put counter in last 8 bytes, big-endian
    for (int i = 0; i < Crypto::COUNTER_SIZE; i++) {
      nonce[Crypto::COUNTER_NONCE_SIZE - Crypto::COUNTER_SIZE + i] = (counter >> (8 * (Crypto::COUNTER_SIZE - 1 - i))) & 0xFF;
    }
  }

  static void onData(Packet::Packet& pkt)
  {
   if (!Handshake::isEstablished()) return;

    // Deserialize counter from big-endian
    uint64_t counter = 0;
    for (size_t i = 0; i < Crypto::COUNTER_SIZE; i++) {
      counter = (counter << 8) | pkt.payload[Crypto::ROLE_SIZE + i];
    }

    // Replay protection
    if (counter <= session.rxCounter) {
      Serial.println("Replay detected");
      return;
    }

    // Compute lengths
    size_t cipherLen = pkt.length - Crypto::ROLE_SIZE - Crypto::COUNTER_SIZE - Crypto::TAG_SIZE;
    const uint8_t* ciphertext = pkt.payload + Crypto::ROLE_SIZE + Crypto::COUNTER_SIZE;
    const uint8_t* tag = pkt.payload + Crypto::ROLE_SIZE + Crypto::COUNTER_SIZE + cipherLen;

    uint8_t senderByte = pkt.payload[0];

    uint8_t senderRole = Handshake::unpackRole(senderByte);

    // Reflection Protection
    uint8_t myRole = Handshake::getRole();

    if (senderRole == myRole) {
      Serial.println("Reflection detected");
      return;
    }

    bool wasCompressed = Handshake::unpackCompressed(senderByte);

    // Rebuild nonce from counter
    uint8_t nonce[Crypto::COUNTER_NONCE_SIZE];

    buildCounterNonce(senderRole, counter, nonce);

    // Decrypt into exact message length
    uint8_t decrypted[65];
    bool ok = Crypto::decrypt(
      session.sessionKey,
      nonce,
      //counter,
      ciphertext,
      cipherLen,
      tag,
      decrypted
    );

    if (!ok) {
      Serial.println("Decrypt failed (tampered)");
      return;
    }

    uint8_t plaintext[Compression::MAX_INPUT_LEN];

    size_t finalLen;

    if (wasCompressed) {
      finalLen = Compression::decompress(decrypted, cipherLen, plaintext, sizeof(plaintext));
      if (finalLen == 0) {
        // Decompression failed — corrupted packet, drop it
        return;
      }
    } else {
      memcpy(plaintext, decrypted, cipherLen);
      finalLen = cipherLen;
    }

    // Update last received counter
    session.rxCounter = counter;

    // Null-terminate exactly at decrypted length
    if (finalLen < sizeof(plaintext))
      plaintext[finalLen] = '\0';

    if (onDataCb)
      onDataCb(plaintext, finalLen);
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
    uint8_t compressed[Compression::WORK_BUFFER_LEN];
    size_t compLen = Compression::compress(data, len, compressed, sizeof(compressed));

    const uint8_t* toEncrypt;
    size_t encryptLen;
    bool wasCompressed;

    if (compLen > 0) {
      toEncrypt = compressed;
      encryptLen = compLen;
      wasCompressed = true;
    } else {
      toEncrypt = data;
      encryptLen = len;
      wasCompressed = false;
    }

    if (encryptLen > 64) return;  // Limit message size

    uint8_t ciphertext[64];        // Max message size
    uint8_t tag[Crypto::TAG_SIZE]; // 16 bytes

    uint64_t counter = session.txCounter++;

    uint8_t senderRole = Handshake::getRole();

    // Build nonce from counter (12 bytes) - uses unpacked role, unaffected by compression
    uint8_t nonce[Crypto::COUNTER_NONCE_SIZE];

    buildCounterNonce(senderRole, counter, nonce);

    // Encrypt using Monocypher
    Crypto::encrypt(
      session.sessionKey,
      //counter
      nonce,
      toEncrypt,
      encryptLen,
      ciphertext,
      tag
    );

    // Build packet
    uint8_t payload[1 + Crypto::COUNTER_SIZE + 64 + Crypto::TAG_SIZE];
    size_t offset = 0;

    // Include Sender Role + Compression Indicator in first byte
    uint8_t senderByte = Handshake::packSenderByte(senderRole, wasCompressed);
    payload[offset++] = senderByte;
    // payload[offset++] = senderRole;

    // Serialize counter as big-endian
    for (int i = 0; i < Crypto::COUNTER_SIZE; i++) {
      payload[offset++] = (counter >> (8 * (Crypto::COUNTER_SIZE - 1 - i))) & 0xFF;
    }

    // Copy ciphertext and tag exactly
    memcpy(payload + offset, ciphertext, encryptLen); offset += encryptLen;
    memcpy(payload + offset, tag, Crypto::TAG_SIZE); offset += Crypto::TAG_SIZE;

    // ---------------- DEBUG: airtime / bitrate / throughput ----------------
    #ifdef DEBUG_AIRTIME
    {
      size_t rawFinalLen  = 1 + Crypto::COUNTER_SIZE + len + Crypto::TAG_SIZE;
      size_t compFinalLen = offset; // what's actually being sent now

      uint32_t airtimeBefore = LoRa::getTimeOnAir(rawFinalLen);
      uint32_t airtimeAfter  = LoRa::getTimeOnAir(compFinalLen);

      float bpsBefore = (rawFinalLen  * 8.0f) / (airtimeBefore / 1e6f);
      float bpsAfter  = (compFinalLen * 8.0f) / (airtimeAfter  / 1e6f);

      float sizeSavedPct    = ((float)(rawFinalLen - compFinalLen) / rawFinalLen) * 100.0f;
      float airtimeSavedPct = ((float)(airtimeBefore - airtimeAfter) / (float)airtimeBefore) * 100.0f;

      Serial.printf(
        "[AIRTIME] plain=%u smaz=%d compressed=%s | pkt_before=%u pkt_after=%u | "
        "airtime_before=%luus airtime_after=%luus | saved_size=%.1f%% saved_airtime=%.1f%% | "
        "bps_before=%.0f bps_after=%.0f\n",
        len, compLen, wasCompressed ? "yes" : "no",
        rawFinalLen, compFinalLen,
        airtimeBefore, airtimeAfter,
        sizeSavedPct, airtimeSavedPct,
        bpsBefore, bpsAfter
      );
    }
    #endif
    // -------------------------------------------------------------------------

    LoRa::enqueuePacket(Packet::DATA, payload, offset);
  }
}