#include "Handshake.h"
#include "../radio/LoRa.h"
#include "../crypto/Crypto.h"
#include "Session.h"

namespace Handshake
{
  Session& session = SessionManager::get();

  void init()
  {
    session.myId = ESP.getEfuseMac();
    Serial.println(session.myId);
  }

  // <========================>
  //   Build / Unpack Methods
  // <========================> 
  static size_t buildHelloPayload(uint8_t* out, uint64_t id, const uint8_t* nonce, const uint8_t* pubkey)
  {
    size_t offset = 0;

    memcpy(out + offset, &id, Crypto::ID_SIZE); offset += Crypto::ID_SIZE;
    memcpy(out + offset, nonce, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;
    memcpy(out + offset, pubkey, Crypto::KEY_SIZE); offset += Crypto::KEY_SIZE;

    return offset;
  }

  static bool unpackHelloPayload(const uint8_t* payload, size_t len, uint64_t& outId, uint8_t* outNonce, uint8_t* outPubKey)
  {
    if (len != (Crypto::ID_SIZE + Crypto::NONCE_SIZE + Crypto::KEY_SIZE)) return false;

    size_t offset = 0;

    memcpy(&outId, payload + offset, Crypto::ID_SIZE); offset += Crypto::ID_SIZE;
    memcpy(outNonce, payload + offset, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;
    memcpy(outPubKey, payload + offset, Crypto::KEY_SIZE); offset += Crypto::KEY_SIZE;

    return true;
  }

  static size_t buildHelloAck(uint8_t* out, const uint8_t* myNonce, const uint8_t* peerNonce)
  {
    size_t offset = 0;

    memcpy(out + offset, myNonce, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;
    memcpy(out + offset, peerNonce, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;

    return offset;
  }

  static bool unpackHelloAck(const uint8_t* payload, size_t len, uint8_t* outPeerNonce, uint8_t* outEchoNonce)
  {
    if (len != (Crypto::NONCE_SIZE * 2)) return false;

    size_t offset = 0;

    memcpy(outPeerNonce, payload + offset, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;
    memcpy(outEchoNonce, payload + offset, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;

    return true;
  }

  static void buildAuthPayload(uint64_t myId, uint64_t peerId, const uint8_t* myNonce, const uint8_t* peerNonce, const uint8_t* sessionKey, uint8_t role, uint8_t* outTag)
  {
    uint8_t buffer[Crypto::ID_SIZE + Crypto::ID_SIZE + Crypto::NONCE_SIZE + Crypto::NONCE_SIZE + 1]; // + 1 for role binding, PREVENTS AGAINST REFLECTION ATTACKS
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

    memcpy(buffer + offset, &firstId, Crypto::ID_SIZE); offset += Crypto::ID_SIZE;
    memcpy(buffer + offset, &secondId, Crypto::ID_SIZE); offset += Crypto::ID_SIZE;

    memcpy(buffer + offset, firstNonce, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;
    memcpy(buffer + offset, secondNonce, Crypto::NONCE_SIZE); offset += Crypto::NONCE_SIZE;

    // Include Role Binding
    buffer[offset++] = role;

    // MAC using sessionKey
    Crypto::blake2bMAC(
      outTag,
      sessionKey,
      buffer,
      offset
    );
  }

  // <==========================>
  //   Send Packet Functions
  // <==========================> 
  static void sendHello()
  {
    Serial.println("Sending Hello");

    // ID = 8 bytes || NONCE = 16 bytes || PUBKEY = 32 bytes
    uint8_t payload[(Crypto::ID_SIZE + Crypto::NONCE_SIZE + Crypto::KEY_SIZE)];

    Crypto::generateNonce(session.myNonce);
    Crypto::generateKeyPair(session.myPrivKey, session.myPubKey);

    size_t len = buildHelloPayload(payload, session.myId, session.myNonce, session.myPubKey);

    LoRa::enqueuePacket(Packet::HELLO, payload, len);
  }

  static void sendHelloAck()
  {
    Serial.println("Sending Hello Ack");

    // RESEND RECEIVED NONCE
    uint8_t ackPayload[Crypto::NONCE_SIZE * 2];

    size_t len = buildHelloAck(ackPayload, session.myNonce, session.peerNonce);

    LoRa::enqueuePacket(Packet::HELLO_ACK, ackPayload, len);
  }

  static void sendAuth()
  {
    Serial.println("Sending Auth");

    Crypto::computeSharedSecret(session.myPrivKey, session.peerPubKey, session.sharedSecret);

    Crypto::deriveSessionKey(session.myId, session.peerId, session.sharedSecret, session.myNonce, session.peerNonce, session.psk, session.sessionKey);

    uint8_t authTag[Crypto::KEY_SIZE];

    buildAuthPayload(
      session.myId,
      session.peerId,
      session.myNonce,
      session.peerNonce,
      session.sessionKey,
      role,
      authTag
    );

    LoRa::enqueuePacket(Packet::AUTH, authTag, Crypto::KEY_SIZE);
  }

  static void sendAuthAck()
  {
    Serial.println("Sending Auth Ack");

    Crypto::computeSharedSecret(session.myPrivKey, session.peerPubKey, session.sharedSecret);

    Crypto::deriveSessionKey(session.myId, session.peerId, session.sharedSecret, session.myNonce, session.peerNonce, session.psk, session.sessionKey);

    uint8_t authTag[Crypto::KEY_SIZE];

    buildAuthPayload(
      session.myId,
      session.peerId,
      session.myNonce,
      session.peerNonce,
      session.sessionKey,
      role,
      authTag
    );

    LoRa::enqueuePacket(Packet::AUTH_ACK, authTag, Crypto::KEY_SIZE);
  }

  static void startCounter()
  {
    session.txCounter = 1;
    session.rxCounter = 0;
  }

  // NEWEST UPDATE
  static void zeroSecrets()
  {
    Crypto::wipe(session.psk, Crypto::PSK_SIZE);

    Crypto::wipe(session.myNonce, Crypto::NONCE_SIZE);
    Crypto::wipe(session.peerNonce, Crypto::NONCE_SIZE);

    Crypto::wipe(session.myPrivKey, Crypto::KEY_SIZE);
    Crypto::wipe(session.myPubKey, Crypto::KEY_SIZE);
    Crypto::wipe(session.peerPubKey, Crypto::KEY_SIZE);

    Crypto::wipe(session.sharedSecret, Crypto::KEY_SIZE);
  }

  // <==========================>
  //   On Packet Received Funcs.
  // <==========================> 
  void onHello(Packet::Packet& pkt)
  {
    bool ok = unpackHelloPayload(pkt.payload, pkt.length, session.peerId, session.peerNonce, session.peerPubKey);

    if (!ok) return;

    bool shouldBeInitiator = session.myId > session.peerId;

    if (!shouldBeInitiator)
    {
      // If we lose the tie breaker, we become the responder
      role = RESPONDER;

      // Small delay to ensure peer is listening
      delay(500);
      sendHelloAck();
      state = WAIT_AUTH;
    }
  }

  void onHelloAck(Packet::Packet& pkt)
  {
    if (role != INITIATOR || state != WAIT_HELLO_ACK) return;

    uint8_t peerNonce[Crypto::NONCE_SIZE];
    uint8_t echoNonce[Crypto::NONCE_SIZE];

    if (!unpackHelloAck(pkt.payload, pkt.length, peerNonce, echoNonce))
    {
      Serial.println("Invalid HELLO_ACK size");
      return;
    }

    // Validate echo
    if (memcmp(echoNonce, session.myNonce, Crypto::NONCE_SIZE) != 0)
    {
      Serial.println("HELLO_ACK nonce mismatch");
      return;
    }

    // Store peer nonce (if not already stored...)
    memcpy(session.peerNonce, peerNonce, Crypto::NONCE_SIZE);

    Serial.println("HELLO_ACK valid");

    // small delay to ensure initiator is listening
    delay(500);

    sendAuth();

    lastSendTime = millis();

    state = WAIT_AUTH_ACK;
  }

  void onAuth(Packet::Packet& pkt)
  {
    if (role != RESPONDER) return;

    // If responder, compute shared secret and session key here
    Crypto::computeSharedSecret(session.myPrivKey, session.peerPubKey, session.sharedSecret);
    Crypto::deriveSessionKey(
      session.myId,
      session.peerId,
      session.sharedSecret,
      session.myNonce,
      session.peerNonce,
      session.psk,
      session.sessionKey
    );

    uint8_t expected[Crypto::KEY_SIZE];

    buildAuthPayload(
      session.myId,
      session.peerId,
      session.myNonce,
      session.peerNonce,
      session.sessionKey,
      INITIATOR,
      expected
    );

    if (Crypto::verify32(expected, pkt.payload) != 0)
    {
      Serial.println("AUTH FAILED");
      return;
    }

    Serial.println("AUTH SUCCESS");

    // always send ACK, even if already established, fixes edge case where AuthAck gets dropped.
    // small delay to ensure initiator is listening
    delay(500);
    sendAuthAck();

    if (state == WAIT_AUTH) 
    {
      state = ESTABLISHED;
      zeroSecrets();
      startCounter();
    }
  }

  void onAuthAck(Packet::Packet& pkt)
  {
    if (role != INITIATOR || state != WAIT_AUTH_ACK) return;

    uint8_t expected[Crypto::KEY_SIZE];

    buildAuthPayload(
      session.myId,
      session.peerId,
      session.myNonce,
      session.peerNonce,
      session.sessionKey,
      RESPONDER,
      expected
    );

    if (Crypto::verify32(expected, pkt.payload) != 0)
    {
      Serial.println("AUTH FAILED");
      return;
    }

    Serial.println("AUTH SUCCESS");

    state = ESTABLISHED;
    zeroSecrets();
    startCounter();
  }

  void start()
  {
    if (state != WAIT_HELLO) return;

    role = INITIATOR;

    sendHello();

    pairingStartTime = millis();
    lastSendTime = millis();

    state = WAIT_HELLO_ACK;
  }

  void update()
  {
    if (state == WAIT_HELLO || state == ESTABLISHED) return;

    uint32_t currentMs = millis();

    if (currentMs - lastSendTime < RETRY_INTERVAL) return;

    // Resend packet based on state
    switch (state)
    {
      case WAIT_HELLO_ACK:
        if (role == INITIATOR)
        {
          Serial.println("Retry HELLO");
          sendHello();
          lastSendTime = currentMs;
        }
        break;

      case WAIT_AUTH:
        if (role == RESPONDER)
        {
          Serial.println("Retry HELLO ACK");
          sendHelloAck();
          lastSendTime = currentMs;
        }
        break;

      case WAIT_AUTH_ACK:
        if (role == INITIATOR)
        {
          Serial.println("Retry AUTH");
          sendAuth();
          lastSendTime = currentMs;
        }
        break;

      default:
        break;
    }
  }

  void reset()
  {
    // Zero out session key
    Crypto::wipe(session.sessionKey, Crypto::KEY_SIZE);

    session.txCounter = 0;
    session.rxCounter = 0;
    pairingStartTime = 0;
    state = WAIT_HELLO;
    role = NONE;
  }

  uint8_t getRole()
  {
    return role;
  }

  void setPSK(const uint8_t* input)
  {
    memcpy(session.psk, input, Crypto::PSK_SIZE);
  }

  uint32_t getPairingStartTime()
  {
    return pairingStartTime;
  }

  bool isEstablished()
  {
    return state == ESTABLISHED;
  }

  uint8_t packSenderByte(uint8_t role, bool compressed) 
  {
    return (role & ROLE_MASK) | (compressed ? COMPRESSED_FLAG : 0);
  }

  uint8_t unpackRole(uint8_t senderByte)
  {
    return senderByte & ROLE_MASK;
  }

  bool unpackCompressed(uint8_t senderByte)
  {
    return (senderByte & COMPRESSED_FLAG) != 0;
  }
}