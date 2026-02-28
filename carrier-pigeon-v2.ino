#include "StateMachine.h"

DeviceState currentState = STATE_SLEEPING;

void setup()
{
    StateMachine::init();
}

void loop()
{
    StateMachine::update(currentState);
}

// // base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// // enable or disable GxEPD2_GFX base class
// #define ENABLE_GxEPD2_GFX 0

// #include <Arduino.h>
// // #include <Wire.h>
// // #include <SPI.h>
// // #include <GxEPD2_BW.h>
// // #include <GxEPD2_3C.h>
// // #include <Fonts/FreeSansBold9pt7b.h>
// // #include "images.h"

// // Battery Pins
// #define VBAT_PIN 1
// #define ADC_CTRL 37

// // Keyboard Pins
// #define SDA_PIN 41
// #define SCL_PIN 42
// #define KB_ADDR 0x5F

// // =====================
// // SPI bus (ESP32-S3)
// // =====================
// SPIClass epdSPI(FSPI);

// // =====================
// // E-Paper SPI pins
// // =====================
// #define EPD_SCK   36
// #define EPD_MOSI  45
// #define EPD_MISO  -1   // not used

// // Control pins
// #define EPD_CS    26
// #define EPD_DC    48
// #define EPD_RST   47
// #define EPD_BUSY  34

// // 3.7'' EPD Module
// GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(GxEPD2_370_GDEY037T03(/*CS=5*/ EPD_CS, /*DC=*/ EPD_DC, /*RES=*/ EPD_RST, /*BUSY=*/ EPD_BUSY)); // GDEY037T03 240x416, UC8253

// #define BAT_X 344
// #define BAT_Y 12
// #define BAT_W 60
// #define BAT_H 26

// #define TB_X 12
// #define TB_Y 202
// #define TB_W 392
// #define TB_H 26

// enum State { SLEEPING, CONFIG, CONNECTING, CONNECTED };

// State currentState = SLEEPING;

// const unsigned long DISPLAY_INTERVAL = 300;
// unsigned long lastDisplayUpdate = 0;

// const unsigned long KB_INTERVAL = 20;  // poll every 20ms
// unsigned long lastKBRead = 0;

// // Update interval (5 seconds)
// const unsigned long BATTERY_INTERVAL = 5000;
// unsigned long lastBatteryRead = 0;

// char keyBuffer[64];
// int bufferIndex = 0;

// void setup()
// {
//   display.init(115200, true, 50, false);

//   pinMode(EPD_BUSY, INPUT);
//   pinMode(EPD_RST, OUTPUT);
  
//   epdSPI.begin(EPD_SCK, EPD_MISO, EPD_MOSI, EPD_CS);

//   // Bind SPI to display (recommended on ESP32-S3)
//   display.epd2.selectSPI(
//     epdSPI,
//     SPISettings(4000000, MSBFIRST, SPI_MODE0)
//   );

//   // Start Keyboard input
//   Wire.begin(SDA_PIN, SCL_PIN);
  
//   display.setRotation(1);

//   drawSleepingScreen();

//   // // helloFullScreenPartialMode();
//   // delay(1000);
//   // if (display.epd2.hasFastPartialUpdate)
//   // {
//   //   // showBatteryUpdate();
//   //   // showPartialUpdate();
//   //   delay(1000);
//   // }
//   // display.hibernate();
// }

// // METHODS NEEDED, 
// // START LOADING SEQUENCE FOR SCREEN, AND THEN START INITIALIZING ALL OTHER STUFF AFTER
// // DRAW MAIN MENU, PARTIAL UPDATE THE BOX INSIDE,
// // FIRST CHANNEL SELECTION SCREEN, THEN CONNECTING SCREEN UNTIL CONNECTED WITGH TIMER, UPDATES EVERY 5
// // THEN CHAT SCREEN WHICH PARTIAL UPDATES WHENEVER A CHAT COMES IN
// // AND A SECONDARY PARTIAL WINDOW THAT UPDATES WHENEDVER KEY OPRESS IS PRESSED IN CHAT SCREEN 

// void drawSleepingScreen() {

//   uint16_t box_x = 6;
//   uint16_t box_y = 6;
//   uint16_t box_w = 404;
//   uint16_t box_h = 228;

//   display.setFullWindow();
//   display.firstPage();
//   do
//   {
//     display.fillScreen(GxEPD_WHITE);
//     display.drawRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
//     display.drawXBitmap(108, 20, logoBitmapData, 200, 200, GxEPD_BLACK);
//   }
//   while (display.nextPage());
// }

// void drawMainUi() {
//   // 416x240

//   display.setFont(&FreeSansBold9pt7b);
//   display.setTextColor(GxEPD_BLACK);

//   uint16_t box_x = 6;
//   uint16_t box_y = 6;
//   uint16_t box_w = 404;
//   uint16_t box_h = 228;

//   uint16_t inner_box_x = 12;
//   uint16_t inner_box_y = 44;
//   uint16_t inner_box_w = 392;
//   uint16_t inner_box_h = 184;

//   display.setFullWindow();
//   display.firstPage();
//   do
//   {
//     display.fillScreen(GxEPD_WHITE);
//     display.drawRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
//     display.drawRect(inner_box_x, inner_box_y, inner_box_w, inner_box_h, GxEPD_BLACK);
//   }
//   while (display.nextPage());
// }

// float readBatteryVoltage() {

//   digitalWrite(ADC_CTRL, HIGH);   // Enable measurement
//   delay(5);                       // Stabilize divider

//   const int samples = 8;
//   long total = 0;

//   for (int i = 0; i < samples; i++) {
//     total += analogReadMilliVolts(VBAT_PIN);
//     delay(2);
//   }

//   digitalWrite(ADC_CTRL, LOW);    // Disable measurement

//   float avgMilliVolts = total / (float)samples;

//   // Reverse voltage divider (× 4.9)
//   const float dividerFactor = 4.9;
//   const float calibrationFactor = 1.0394;

//   float voltage = (avgMilliVolts * dividerFactor * calibrationFactor) / 1000.0;

//   return voltage;
// }

// int voltageToPercent(float v) {

//   // Clamp extremes
//   if (v >= 4.20) return 100;
//   if (v <= 3.20) return 0;

//   // Piecewise Li-ion curve approximation
//   if (v > 4.0)
//     return 80 + (v - 4.0) * 100;     // 4.0–4.2V

//   if (v > 3.8)
//     return 60 + (v - 3.8) * 100;     // 3.8–4.0V

//   if (v > 3.6)
//     return 30 + (v - 3.6) * 150;     // 3.6–3.8V

//   return (v - 3.2) * 75;             // 3.2–3.6V
// }

// void sendMessage() {

// }

// void hibernate() {
//   memset(keyBuffer, 0, sizeof(keyBuffer));
//   bufferIndex = 0;
//   currentState = SLEEPING;
//   drawSleepingScreen();
// }

// void handleChar(char c) {
//   switch (c) {
//     case 13:
//       if (bufferIndex > 0) {
//         Serial.println("Enter");
//       }
//       break;

//     case 8: 
//       if (bufferIndex > 0) {
//         bufferIndex--;
//         keyBuffer[bufferIndex] = '\0';
//       }
//       break;

//     case 27:
//       //do turn off stuff
//       Serial.println("Hibernate");
//       hibernate();
//       return;

//     case 180:
//       Serial.println("LEFT");
//       break;

//     case 181:
//       Serial.println("UP");
//       break;

//     case 182:
//       Serial.println("DOWN");
//       break;

//     case 183:
//       Serial.println("RIGHT");
//       break;

//     default:
//       keyBuffer[bufferIndex++] = (char)c;
//       keyBuffer[bufferIndex] = '\0';
//       break;
//   }
// }

// void powerOn() {
//   currentState = CONFIG;

//   // Battery ADC Read Setup
//   analogReadResolution(12);

//   pinMode(ADC_CTRL, OUTPUT);
//   digitalWrite(ADC_CTRL, LOW); // Keep OFF until needed

//   drawMainUi();
// }

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

// void updateBattery()
// {
//   if (millis() - lastBatteryRead >= BATTERY_INTERVAL) {
//     lastBatteryRead = millis();

//     float batteryVoltage = 0.0;
//     int batteryPercent = 0;

//     batteryVoltage = readBatteryVoltage();
//     batteryPercent = voltageToPercent(batteryVoltage);

//     display.setPartialWindow(BAT_X, BAT_Y, BAT_W, BAT_H);
//     display.firstPage();
//     do
//     {
//       display.fillRect(BAT_X, BAT_Y, BAT_W, BAT_H, GxEPD_WHITE);
//       display.drawRect(BAT_X, BAT_Y, BAT_W, BAT_H, GxEPD_BLACK);

//       display.setCursor(BAT_X + 5, BAT_Y + 18);
//       display.print(batteryPercent);
//       display.print("%");
//     }
//     while (display.nextPage());
//   }
// }

// void updateDisplay()
// {
//   if (millis() - lastDisplayUpdate < DISPLAY_INTERVAL)
//     return;

//   lastDisplayUpdate = millis();

//   display.setPartialWindow(TB_X, TB_Y, TB_W, TB_H);
//   display.firstPage();
//   do
//   {
//     display.fillRect(TB_X, TB_Y, TB_W, TB_H, GxEPD_WHITE);
//     display.drawRect(TB_X, TB_Y, TB_W, TB_H, GxEPD_BLACK);

//     display.setCursor(TB_X + 5, TB_Y + 18);
//     display.print(keyBuffer);
//   }
//   while (display.nextPage());
// }

// void pollWakeupKeypress() {
//   // Enable timer wake (10ms)
//   esp_sleep_enable_timer_wakeup(100000); // 10,000 µs = 10ms
//   esp_light_sleep_start();

//   // After wake — check keyboard
//   Wire.requestFrom(0x5F, 1);  // CardKB default address

//   if (Wire.available()) {
//       uint8_t key = Wire.read();
//       if (key != 0) {
//         powerOn();
//       }
//   }
// }

// void loop()
// {
//   if (currentState == SLEEPING) {
//     pollWakeupKeypress();
//     return;  // prevent fallthrough
//   }

//   // ACTIVE STATE ONLY
//   updateBattery();
//   updateDisplay();
//   pollKeyboard();

//   // Your other logic runs freely here:
//   // LoRa receive
//   // Keypress detection
//   // Display updates
// }