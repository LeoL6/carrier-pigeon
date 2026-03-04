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
  bool isMessageAvailable();
  uint16_t getMessage(uint8_t* buffer);
}