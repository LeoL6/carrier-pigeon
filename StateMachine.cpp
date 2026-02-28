#include <Arduino.h>
#include "StateMachine.h"
#include "Display.h"
// #include "L.h"
// #include "Battery.h"
#include "Keyboard.h"
#include <esp_sleep.h>

static DeviceState previousState = STATE_SLEEPING;

// Forward declarations
static void enterState(DeviceState state);
static void runSleeping(DeviceState &state);
static void runConfig(DeviceState &state);
static void runConnecting(DeviceState &state);
static void runConnected(DeviceState &state);

static char keyBuffer[64];
static int bufferIndex = 0;

void StateMachine::init()
{
    Keyboard::init();
    Display::init();
    // LoRa::init();
    // Battery::init();

    enterState(STATE_SLEEPING);
}

void StateMachine::update(DeviceState &state)
{
    // Detect state change
    if (state != previousState)
    {
        enterState(state);
        previousState = state;
    }

    // Run current state logic
    switch (state)
    {
        case STATE_SLEEPING:
            runSleeping(state);
            break;

        case STATE_CONFIG:
            runConfig(state);
            break;

        case STATE_CONNECTING:
            runConnecting(state);
            break;

        case STATE_CONNECTED:
            runConnected(state);
            break;

        default:
            break;
    }
}

static void enterState(DeviceState state)
{
    switch (state)
    {
        case STATE_SLEEPING:
            Display::drawSleepingScreen();
            break;

        default:
            Display::drawActiveScreen();
            break;
    }
}

// static void updateBattery()
// {
//     static unsigned long lastUpdate = 0;
//     const unsigned long interval = 3000;

//     if (millis() - lastUpdate >= interval)
//     {
//         lastUpdate = millis();

//         float voltage = Battery::readVoltage();
//         Display::updateBattery(voltage);
//     }
// }

static void runSleeping(DeviceState &state)
{
    esp_sleep_enable_timer_wakeup(50000); // 50ms
    esp_light_sleep_start();

    char key;
    if (Keyboard::read(key))
    {
        state = STATE_CONNECTED;
        // Wake on any key
        // state = STATE_CONFIG;
        // state = STATE_CONNECTED;
    }
}

// void pollWakeupKeypress() {
//   // Enable timer wake (10ms)
//   esp_sleep_enable_timer_wakeup(100000); // 10,000 µs = 10ms
//   esp_light_sleep_start();

//   // After wake — check keyboard
//   Wire.requestFrom(0x5F, 1);  // CardKB default address

//   if (Wire.available()) {
//       uint8_t key = Wire.read();
//       if (key != 0) {
//         powerOn();
//       }
//   }
// }

static void runConfig(DeviceState &state) {}

static void runConnecting(DeviceState &state) {}

static void runConnected(DeviceState &state)
{
    // Keyboard::update();
    // lora_manager::update();

    // Handle keyboard input
    char key;
    if (Keyboard::read(key))
    {
        // char key = Keyboard::getKey();

        switch (key)
        {
            case 13:
                if (bufferIndex > 0) 
                {
                    Serial.println("Enter");
                    // lora_manager::sendMessage(keyBuffer);
                    bufferIndex = 0;
                    keyBuffer[0] = '\0';
                }
                break;

            case 8: 
                if (bufferIndex > 0) 
                {
                    bufferIndex--;
                    keyBuffer[bufferIndex] = '\0';
                }
                break;

            case 27:
                //do turn off stuff
                Serial.println("Hibernate");
                state = STATE_SLEEPING;
                return;

            case 180:
                Serial.println("LEFT");
                break;

            case 181:
                Serial.println("UP");
                break;

            case 182:
                Serial.println("DOWN");
                break;

            case 183:
                Serial.println("RIGHT");
                break;

            default:
                // if (key == 0) break;
                if (bufferIndex < sizeof(keyBuffer) - 1)
                {
                    keyBuffer[bufferIndex++] = (char)key;
                    keyBuffer[bufferIndex] = '\0';
                    Serial.printf("Buffer: %s\n", keyBuffer);
                }
                break;
        }
    }

    // // Update battery periodically
    // // float voltage = Battery::readVoltage();
    // // Display::updateBattery(voltage);
    // updateBattery();

    // // Update UI with buffer content
    Display::updateInputBuffer(keyBuffer);
}
