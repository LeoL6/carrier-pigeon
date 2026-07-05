#pragma once

namespace LoRa
{
  // <========================>
  //   Constants
  // <========================>
  using ReceiveCallback = void(*)(uint8_t*, size_t);
  static constexpr int BUFFER_SIZE = 128;
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
  void clearQueue();
  bool enqueuePacket(uint8_t type, const uint8_t* payload, uint8_t len);

  // <========================>
  //   Debug Functions
  // <========================>
  uint32_t getTimeOnAir(size_t payloadLen);
}