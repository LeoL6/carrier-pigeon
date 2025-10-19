#include <WiFi.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#include "webserver_setup.h"
#include "websocket_handler.h"

WebServer server(80);
WebSocketsServer webSocket(81);

void setup() {
  Serial.begin(115200);
  WiFi.softAP("Carrier Pigeon", "password");
  Serial.println(WiFi.softAPIP());

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed!");
    return;
  }

  initWebServer();
  initWebSocket();

  Serial.println("Server + WebSocket running with LittleFS");
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
