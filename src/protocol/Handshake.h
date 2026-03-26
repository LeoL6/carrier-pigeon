#pragma once
#include <Arduino.h>
#include "Packet.h"

namespace Handshake
{
  // <========================>
  //   Handshake States
  // <========================> 
  enum State
  {
    WAIT_HELLO,
    WAIT_HELLO_ACK,
    WAIT_AUTH,
    WAIT_AUTH_ACK,
    ESTABLISHED
  };

  // <==========================>
  //   Current Handshake State
  // <==========================> 
  static State state = WAIT_HELLO;

  // <========================>
  //   Handshake Roles
  // <========================> 
  enum Role : uint8_t
  {
    NONE = 0,
    INITIATOR = 1,
    RESPONDER = 2
  };

  // <==========================>
  //   Current Handshake Role
  // <==========================> 
  static Role role = NONE;

  // <==========================>
  //   Pairing Start Time
  // <==========================> 
  static uint32_t pairingStartTime;

  // <==========================>
  //   Retry Timing Variables
  // <==========================> 
  const uint32_t RETRY_INTERVAL = 5000;
  static uint32_t lastSendTime;

  // <============================>
  //   Application Layer Functions
  // <============================> 
  void init();
  void start();
  void update();
  void reset();

  // <==========================>
  //   Handshake State Functions
  // <==========================> 
  void onHello(Packet::Packet& pkt);
  void onHelloAck(Packet::Packet& pkt);
  void onAuth(Packet::Packet& pkt);
  void onAuthAck(Packet::Packet& pkt);

  // <========================>
  //   Helper Functions
  // <========================> 
  void setPSK(const uint8_t* input);
  uint32_t getPairingStartTime();
  bool isEstablished();
}