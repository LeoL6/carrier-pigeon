#pragma once

namespace LoRa
{
  // <========================>
  //   Constants
  // <========================>
  static constexpr int BUFFER_SIZE = 256;

  void init();
  void update();

  // <========================>
  //   Message Functions
  // <========================>
  void sendMessage(const char* msg);
  bool isMessageAvailable();
  uint16_t getMessage(uint8_t* buffer);
}