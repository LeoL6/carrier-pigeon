#include <Wire.h>
#include "Keyboard.h"

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

  bool readRaw(char& key)
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

  bool getEvent(KeyEvent& event)
  {
      char key;

      if (!readRaw(key))
          return false;

      switch (key)
      {
          case 13:
              event.type = KeyEventType::ENTER;
              break;

          case 8:
              event.type = KeyEventType::BACKSPACE;
              break;

          case 27:
              event.type = KeyEventType::ESCAPE;
              break;

          // case 180:
          //     event.type = KeyEventType::ARROW_LEFT;
          //     break;

          // case 181:
          //     event.type = KeyEventType::ARROW_UP;
          //     break;

          // case 182:
          //     event.type = KeyEventType::ARROW_DOWN;
          //     break;

          // case 183:
          //     event.type = KeyEventType::ARROW_RIGHT;
          //     break;

          default:
              event.type = KeyEventType::CHARACTER;
              event.character = key;
              break;
      }

      return true;
  }
}
