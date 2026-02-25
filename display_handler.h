// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

#include <SPI.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "images.h"

// =====================
// Battery Pins
// =====================
#define VBAT_PIN 1
#define ADC_CTRL 37

// =====================
// SPI bus (ESP32-S3)
// =====================
SPIClass epdSPI(FSPI);

// =====================
// E-Paper SPI pins
// =====================
#define EPD_SCK   36
#define EPD_MOSI  45
#define EPD_MISO  -1   // not used

// =====================
// Control pins
// =====================
#define EPD_CS    26
#define EPD_DC    48
#define EPD_RST   47
#define EPD_BUSY  34

// =====================
// 3.7'' EPD Module
// =====================
GxEPD2_BW<GxEPD2_370_GDEY037T03, GxEPD2_370_GDEY037T03::HEIGHT> display(GxEPD2_370_GDEY037T03(/*CS=5*/ EPD_CS, /*DC=*/ EPD_DC, /*RES=*/ EPD_RST, /*BUSY=*/ EPD_BUSY)); // GDEY037T03 240x416, UC8253


// =====================
// ELEMENT SIZES
// =====================
#define BAT_X 344
#define BAT_Y 12
#define BAT_W 60
#define BAT_H 26

#define TB_X 12
#define TB_Y 202
#define TB_W 392
#define TB_H 26

