#include <Arduino.h>
#include "StateMachine.h"
#include <esp_sleep.h>

// <====================>
//   Service Modules
// <====================>
#include "../ui/Display.h"
#include "../radio/LoRa.h"
#include "../services/Battery.h"
#include "../input/Keyboard.h"

// ON FINAL PUSH REMOVE THIS COMMENT AND REMOVE ALL SERIAL PRINTLNS

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
    LoRa::init();

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
            memset(keyBuffer, 0, sizeof(keyBuffer));
            bufferIndex = 0;
            break;

        default:
            Serial.println("ENTERING");
            Serial.println(state);
            Battery::wakeUp();
            Display::drawActiveScreen();
            break;
    }
}

// This interval is placed inside of StateMachine because it is application level...
// Whereas: the interval for updateInputBuffer is inside of the Display module because updating too quick is a hardware constraint, not application choice.
static void updateBattery()
{
    static unsigned long lastUpdate = 0;
    const unsigned long interval = 3000;

    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();

        int percentage = Battery::readPercentage();
        bool charging = Battery::isUsbConnected();
        Display::updateBattery(percentage, charging);
    }
}

static void updateChat()
{
    static unsigned long lastUpdate = 0;
    const unsigned long interval = 50;

    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();

        uint8_t buffer[256];

        if (LoRa::isMessageAvailable())
        {
            uint16_t size = LoRa::getMessage(buffer);

            if (size < LoRa::BUFFER_SIZE)
                buffer[size] = '\0';

            Serial.println((char*)buffer);

            // Display::showMessage((char*)buffer);
        }
    }
}

static void runSleeping(DeviceState &state)
{
    esp_sleep_enable_timer_wakeup(50000); // 50ms
    esp_light_sleep_start();

    KeyEvent event;

    if (Keyboard::getEvent(event))
    {
        // Wake on any key
        state = STATE_CONNECTED;
        // state = STATE_CONFIG;
    }
}

static void sendMessage()
{
    // if (bufferIndex < sizeof(keyBuffer) - 1)
    // {
    //     keyBuffer[bufferIndex++] = (char)key;
    //     keyBuffer[bufferIndex] = '\0';
    //     Serial.printf("Buffer: %s\n", keyBuffer);
    // }
}

static void handleInput(const KeyEvent& event, DeviceState &state)
{
    switch (event.type)
    {
        case KeyEventType::CHARACTER:
            // Messages::appendInputChar(event.character);
            break;

        case KeyEventType::BACKSPACE:
            // Messages::removeInputChar();
            break;

        case KeyEventType::ENTER:
            // sendMessage();
            break;

        case KeyEventType::ESCAPE:
            switch (state)
            {
                case STATE_CONNECTED:
                    state = STATE_CONFIG;
                    break;

                case STATE_CONFIG:
                    state = STATE_SLEEPING;
                    break;
            }
            break;

        // case KeyEventType::ARROW_UP:
        //     messages.scrollUp();
        //     break;

        // case KeyEventType::ARROW_DOWN:
        //     messages.scrollDown();
        //     break;

        default:
            break;
    }
    // char key;
    // if (Keyboard::read(key))
    // {
    //     switch (key)
    //     {
    //         case 13:
    //             if (bufferIndex > 0) 
    //             {
    //                 Serial.println("Enter");
    //                 // lora_manager::sendMessage(keyBuffer);
    //                 bufferIndex = 0;
    //                 keyBuffer[0] = '\0';
    //             }
    //             break;

    //         case 8: 
    //             if (bufferIndex > 0) 
    //             {
    //                 bufferIndex--;
    //                 keyBuffer[bufferIndex] = '\0';
    //             }
    //             break;

    //         case 27:
    //             //do turn off stuff
    //             Serial.println("Hibernate");
    //             state = STATE_SLEEPING;
    //             return;

    //         case 180:
    //             Serial.println("LEFT");
    //             break;

    //         case 181:
    //             Serial.println("UP");
    //             break;

    //         case 182:
    //             Serial.println("DOWN");
    //             break;

    //         case 183:
    //             Serial.println("RIGHT");
    //             break;

    //         default:
    //             if (bufferIndex < sizeof(keyBuffer) - 1)
    //             {
    //                 keyBuffer[bufferIndex++] = (char)key;
    //                 keyBuffer[bufferIndex] = '\0';
    //                 Serial.printf("Buffer: %s\n", keyBuffer);
    //             }
    //             break;
    //     }
    // }
}

static void runConfig(DeviceState &state) 
{
    KeyEvent event;

    if (Keyboard::getEvent(event))
    {
        handleInput(event, state);
    }

    // Update battery periodically (Every 3s)
    updateBattery();
}

static void runConnecting(DeviceState &state) {}

static void runConnected(DeviceState &state)
{
    LoRa::update();

    // Handle keyboard input

    KeyEvent event;

    if (Keyboard::getEvent(event))
    {
        handleInput(event, state);
    }

    // Update battery periodically (Every 3s)
    updateBattery();

    // Update Chat Buffer periodically ( Every 20ms )
    updateChat();

    // Update UI with buffer content periodically (Every 20ms)
    Display::updateInputBuffer(keyBuffer);
}
