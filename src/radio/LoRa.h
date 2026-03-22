#pragma once

namespace LoRa
{
  // <========================>
  //   Constants
  // <========================>
  using ReceiveCallback = void(*)(uint8_t*, size_t);
  static constexpr int BUFFER_SIZE = 256;
  static constexpr int QUEUE_SIZE = 8;

  // <========================>
  //   Structs
  // <========================>
  struct QueuedPacket
  {
    uint8_t type;
    uint8_t payload[BUFFER_SIZE];
    uint8_t length;
  };

  // <========================>
  //   Variables
  // <========================>
  static QueuedPacket sendQueue[QUEUE_SIZE];
  static volatile uint8_t queueHead = 0;
  static volatile uint8_t queueTail = 0;

  // <========================>
  //   Setup Functions
  // <========================>
  void init();
  void update();
  void setReceiveCallback(ReceiveCallback cb);

  // <========================>
  //   Packet Functions
  // <========================>
  bool enqueuePacket(uint8_t type, const uint8_t* payload, uint8_t len);
  // void sendPacket(uint8_t type, const uint8_t* payload, size_t payloadLen);
  // bool isMessageAvailable();
  // uint16_t getMessage(uint8_t* buffer);
}