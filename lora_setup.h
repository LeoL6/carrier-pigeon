#pragma once
#include "LoRaWan_APP.h"
#include "Arduino.h"

// LoRa config for North America
#define RF_FREQUENCY        915E6   // Hz
#define TX_OUTPUT_POWER     14      // dBm
#define LORA_BANDWIDTH      0       // 0:125kHz, 1:250kHz, 2:500kHz
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE     1       // 4/5
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

// Forward declarations
void initLoRa();
void sendLoRaMessage(const String &msg);
void onTxDone();
void onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

// Radio event structure
extern RadioEvents_t RadioEvents;

// Setup function
void initLoRa() {
    Serial.println("Initializing Heltec LoRa...");
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

    // Attach event handlers
    RadioEvents.TxDone = onTxDone;
    RadioEvents.RxDone = onRxDone;

    // Init radio
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);

    // Configure transmission
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
                    
    // Configure reception
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    0, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

    Serial.println("LoRa initialized (Heltec native driver)");
    Radio.Rx(0);  // Start listening immediately
}

// Send LoRa message (UTF-8 safe)
void sendLoRaMessage(const String &msg) {
    uint8_t buffer[255];
    uint16_t len = msg.length();
    msg.getBytes(buffer, len + 1);

    Radio.Send(buffer, len);
    Serial.println("LoRa TX -> " + msg);
}
