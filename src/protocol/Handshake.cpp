#include "Handshake.h"
#include "../radio/LoRa.h"

namespace Handshake
{
  void init()
  {
    // myId = ESP.getEfuseMac();
    myId = 1;
  }

  // <==========================>
  //   Send Packet Functions
  // <==========================> 
  static void sendHello()
  {
    Serial.println("Sending Hello");
    LoRa::enqueuePacket(
      Packet::HELLO, 
      (uint8_t*)&myId,
      sizeof(myId)
    );
  }

  static void sendHelloAck()
  {
    Serial.println("Sending Hello Ack");
    LoRa::enqueuePacket(Packet::HELLO_ACK, nullptr, 0);
  }

  static void sendAuth()
  {
    Serial.println("Sending Auth");
    LoRa::enqueuePacket(Packet::AUTH, nullptr, 0);
  }

  static void sendAuthAck()
  {
    Serial.println("Sending Auth Ack");
    LoRa::enqueuePacket(Packet::AUTH_ACK, nullptr, 0);
  }

  // <==========================>
  //   On Packet Received Funcs.
  // <==========================> 
  void onHello(Packet::Packet& pkt)
  {
    uint64_t peerId;
    memcpy(&peerId, pkt.payload, sizeof(peerId));

    bool shouldBeInitiator = myId > peerId;

    if (!shouldBeInitiator)
    {
      // If we lose the tie breaker, we become the responder
      role = RESPONDER;

      sendHelloAck();
      state = WAIT_AUTH;
    }
  }

  void onHelloAck(Packet::Packet& pkt)
  {
    if (role != INITIATOR || state != WAIT_HELLO_ACK) return;

    sendAuth();

    lastSendTime = millis();

    state = WAIT_AUTH_ACK;
  }

  void onAuth(Packet::Packet& pkt)
  {
    if (role != RESPONDER || state != WAIT_AUTH) return;

    sendAuthAck();

    state = ESTABLISHED;
  }

  void onAuthAck(Packet::Packet& pkt)
  {
    if (role != INITIATOR || state != WAIT_AUTH_ACK) return;

    state = ESTABLISHED;
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
    pairingStartTime = 0;
    state = WAIT_HELLO;
    role = NONE;
  }

  uint32_t getPairingStartTime()
  {
    return pairingStartTime;
  }

  bool isEstablished()
  {
    return state == ESTABLISHED;
  }
}