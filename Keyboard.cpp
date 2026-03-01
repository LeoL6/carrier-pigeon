#include <Wire.h>

namespace Keyboard {
  // <===================>
  //   Keyboard Pins
  // <===================>
  static constexpr int PIN_SDA = 41;
  static constexpr int PIN_SCL = 42;

  // <===================>
  //   Keyboard Addresses
  // <===================>
  static constexpr uint8_t ADDR_KB = 0x5F;

  void init()
  {
    // Start Keyboard input
    Wire.begin(PIN_SDA, PIN_SCL);
  }

  bool read(char& key)
  {
    if (!Wire.requestFrom(ADDR_KB, 1))
        return false;

    if (!Wire.available())
        return false;

    char c = Wire.read();

    if (c == 0)
        return false;  // ignore "no key"

    key = c;
    return true;
  }
}
