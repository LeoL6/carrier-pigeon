#pragma once
#include <WebServer.h>
#include <LittleFS.h>

extern WebServer server;

void handleFile(String path, String type) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    server.send(404, "text/plain", "File Not Found");
    return;
  }
  server.streamFile(file, type);
  file.close();
}

void initWebServer() {
  server.on("/", []() { handleFile("/index.html", "text/html"); });
  server.on("/style.css", []() { handleFile("/style.css", "text/css"); });
  server.on("/script.js", []() { handleFile("/script.js", "text/javascript"); });
  server.onNotFound([]() { server.send(404, "text/plain", "File Not Found"); });
  server.begin();
}
