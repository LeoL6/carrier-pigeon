#include "lora_setup.h"
#include "lora_handler.h"
#include "webserver_setup.h"
#include "websocket_setup.h"

void setup() {
    Serial.begin(115200);

    initWebServer();
    initWebSocket();
    initLoRa();

    Serial.println("Server + WebSocket + LoRa running");
}

void loop() {
    handleWebSocket();
    handleLoRa();
}
