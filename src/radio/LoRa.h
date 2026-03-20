#pragma once

namespace LoRa
{
  // <========================>
  //   Constants
  // <========================>
  using ReceiveCallback = void(*)(uint8_t*, size_t);
  static constexpr int BUFFER_SIZE = 256;

  // <========================>
  //   Setup Functions
  // <========================>
  void init();
  void update();
  void setReceiveCallback(ReceiveCallback cb);

  // <========================>
  //   Packet Functions
  // <========================>
  void sendPacket(uint8_t type, const uint8_t* payload, size_t payloadLen);
  // bool isMessageAvailable();
  // uint16_t getMessage(uint8_t* buffer);
}