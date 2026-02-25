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