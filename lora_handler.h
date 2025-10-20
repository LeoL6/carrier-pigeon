#pragma once
#include "lora_setup.h"
#include <WebSocketsServer.h>

extern WebSocketsServer webSocket;
RadioEvents_t RadioEvents;

unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 5000; // 5s

void onTxDone() {
    Serial.println("LoRa TX done");
    Radio.Rx(0);
}

void onRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    String incoming;
    for (int i = 0; i < size; i++) incoming += (char)payload[i];

    Serial.printf("LoRa RX → %s (RSSI: %d dBm)\n", incoming.c_str(), rssi);

    if (incoming == "ACK") {
        Serial.println("Received heartbeat");
    }

    // Forward to all WebSocket clients
    webSocket.broadcastTXT(incoming);

    Radio.Rx(0);
}

void handleLoRa() {
    Radio.IrqProcess(); // Required occasionally for RX/TX Callbacks to be triggered

    unsigned long now = millis();
    if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        lastHeartbeat = now;
        sendLoRaMessage("ACK");
        Serial.println("Heartbeat sent");
    }
}
