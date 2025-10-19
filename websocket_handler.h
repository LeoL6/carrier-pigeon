#pragma once
#include <WebSocketsServer.h>

extern WebSocketsServer webSocket;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type == WStype_CONNECTED) {
    if(webSocket.connectedClients() > 1) {
      webSocket.disconnect(num); // disconnect the new client, if someone tries to join an already active chat
    }
  }
  if (type == WStype_TEXT) {
    String msg = "{\"type\":\"received\",\"text\":\"" + String((char*)payload) + "\"}";
    if(msg == "{\"type\":\"heartbeat\"}") {
      // webSocket.sendTXT(num, "{\"type\":\"ack\"}"); // send ACK to sender
    } else {
      // normal message handling
      webSocket.broadcastTXT(msg);
    }
    Serial.println("WS Received: " + msg);
    webSocket.broadcastTXT(msg);
  }
}

void initWebSocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
