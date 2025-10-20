#pragma once
#include <WebSocketsServer.h>
#include "lora_setup.h" // for sendLoRaMessage()

// Create WebSocket server (port 81)
WebSocketsServer webSocket = WebSocketsServer(81);

// Forward declaration
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void initWebSocket() {
    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
    Serial.println("WebSocket server started");
}

void handleWebSocket() {
    webSocket.loop();
}

// Event handler
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED: {
          if(webSocket.connectedClients() > 1) {
            webSocket.disconnect(num); // disconnect the new client, if someone tries to join an already active chat
          }
          IPAddress ip = webSocket.remoteIP(num);
          Serial.printf("[WS] Client connected: %s\n", ip.toString().c_str());
          break;
        }
        case WStype_TEXT: {
            String msg = String((char*)payload);
            Serial.printf("[WS] From client: %s\n", msg.c_str());
            sendLoRaMessage(msg); // Forward message to LoRa
            break;
        }
        case WStype_DISCONNECTED: {
            Serial.printf("[WS] Client %u disconnected\n", num);
            break;
        }
        default:
            break;
    }
}

