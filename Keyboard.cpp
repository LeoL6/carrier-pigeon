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

  // void pollKeyboard() {

  //   if (millis() - lastKBRead < KB_INTERVAL) return;
  //   lastKBRead = millis();

  //   Wire.requestFrom(KB_ADDR, 1);

  //   // TO GET BYTE VALUE AND RAW DIGIT VALUE
  //   // if (Wire.available()) {
  //   //   uint8_t raw = Wire.read();
  //   //   Serial.printf("Raw: %d (0x%02X)\n", raw, raw);
  //   // } 

  //   if (Wire.available()) {
  //     char c = Wire.read();

  //     if (c != 0) {
  //       // if (currentState == SLEEPING) {
  //       //   power
  //       //   return;
  //       // }

  //       if (bufferIndex < sizeof(keyBuffer) - 1) {
  //         handleChar(c);
  //       }

  //       // Serial.printf("Buffer: %s\n", keyBuffer);
  //     }
  //   }
  // }
}
