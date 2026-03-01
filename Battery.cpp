#include <Arduino.h>
#include <Wire.h>

namespace Battery
{
  // <===================>
  //    Battery Pins
  // <===================>
  static constexpr int PIN_VBAT = 1;
  static constexpr int ADC_CTRL = 37;

  void wakeUp()
  {
    // Battery ADC Read Setup
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_VBAT, ADC_11db);

    pinMode(ADC_CTRL, OUTPUT);
    // Keep OFF until needed
    digitalWrite(ADC_CTRL, LOW);
  }

  float readVoltage() 
  {
    digitalWrite(ADC_CTRL, HIGH);   // Enable measurement
    delay(5);                       // Stabilize divider

    const int samples = 8;
    long total = 0;

    for (int i = 0; i < samples; i++) {
      total += analogReadMilliVolts(PIN_VBAT);
      delay(2);
    }

    digitalWrite(ADC_CTRL, LOW);    // Disable measurement

    float avgMilliVolts = total / (float) samples;

    // Reverse voltage divider (× 4.9)
    const float dividerFactor = 4.9;
    const float calibrationFactor = 1.0394;

    float voltage = (avgMilliVolts * dividerFactor * calibrationFactor) / 1000.0;

    return voltage;
  }

  int voltageToPercentage(float v) 
  {
    // Clamp extremes
    if (v >= 4.20) return 100;
    if (v <= 3.20) return 0;

    // Piecewise Li-ion curve approximation
    if (v > 4.0)
      return 80 + (v - 4.0) * 100;     // 4.0–4.2V

    if (v > 3.8)
      return 60 + (v - 3.8) * 100;     // 3.8–4.0V

    if (v > 3.6)
      return 30 + (v - 3.6) * 150;     // 3.6–3.8V

    return (v - 3.2) * 75;             // 3.2–3.6V
  }

  int readPercentage()
  {
    float v = readVoltage();
    return voltageToPercentage(v);
  }
}