#pragma once
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Create Async HTTP server
AsyncWebServer server(80);

inline void serveAsyncRoutes() {
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Route to load .js files
  server.on("/nacl-fast.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/nacl-fast.min.js", "text/javascript");
  });
  server.on("/cryptoHandler.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/cryptoHandler.js", "text/javascript");
  });
  server.on("/trafficHandler.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/trafficHandler.js", "text/javascript");
  });
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/main.js", "text/javascript");
  });
}

// Initialize the AsyncWebServer
inline void initWebServer() {
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS mount failed!");
    return;
  }

  WiFi.softAP("Carrier Pigeon One", "password123");
  Serial.println(WiFi.softAPIP());

  serveAsyncRoutes();

  server.begin();
  Serial.println("Async HTTP server started");
}
