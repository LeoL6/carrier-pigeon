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

// oid updateDisplay()
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