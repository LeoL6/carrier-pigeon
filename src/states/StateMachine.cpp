#include <Arduino.h>
#include "StateMachine.h"
#include <esp_sleep.h>

// <====================>
//   Service Modules
// <====================>
#include "../ui/Display.h"
#include "../radio/LoRa.h"
#include "../system/Battery.h"
#include "../input/Keyboard.h"

// <===================>
//   Domain Modules
// <===================>
#include "../message/Messages.h"
#include "../protocol/Packet.h"
#include "../protocol/Protocol.h"

// ON FINAL PUSH REMOVE THIS COMMENT AND REMOVE ALL SERIAL PRINTLNS

static DeviceState previousState = STATE_SLEEPING;

// <======================>
//   Forward declarations
// <======================>
static void enterState(DeviceState state);
static void runSleeping(DeviceState &state);
static void runConfig(DeviceState &state);
static void runPairing(DeviceState &state);
static void runConnected(DeviceState &state);

// <======================>
//   Radio Callback
// <======================>
static void onRadioPacket(uint8_t* data, size_t len)
{
    Packet::Packet pkt;

    if (!Packet::parse(data, len, pkt)) return;

    Protocol::onReceive(pkt);
}

void StateMachine::init()
{
    Keyboard::init();
    Display::init();
    LoRa::init();
    LoRa::setReceiveCallback(onRadioPacket);

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

        case STATE_PAIRING:
            runPairing(state);
            break;

        case STATE_CONNECTED:
            runConnected(state);
            break;

        default:
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

static void sendMessage()
{
    if (!Messages::isInputBufferEmpty())
    {
        const char* inputBuffer = Messages::getInputBuffer();

        Message msg;
        strncpy(msg.text, inputBuffer, MAX_MESSAGE_LEN);
        msg.outgoing = true;

        Messages::push(msg);

        size_t len = strlen(inputBuffer);

        if (len > LoRa::BUFFER_SIZE - 2)
            len = LoRa::BUFFER_SIZE - 2;

        LoRa::sendPacket(
            Packet::DATA,
            (const uint8_t*)inputBuffer,
            strlen(inputBuffer)
        );
        
        Messages::clearInputBuffer();
        Display::drawMessages();
    }
}

static void enterPairing() {
    const char* inputBuffer = Messages::getInputBuffer();

    // SEND THIS INPUT BUFFER OFF TO PROTOCOL OR HANDSHAKE RELATED FUNCTION OR CRYPTO?

    // Message msg;
    // strncpy(msg.text, inputBuffer, MAX_MESSAGE_LEN);
    // msg.outgoing = true;

    // Messages::push(msg);
    // LoRa::sendMessage(inputBuffer);

    Messages::clearInputBuffer();

    StateMachine::pairingStartTime = millis();
}

static void updatePairingTimer()
{
    static unsigned long lastUpdate = 0;
    const unsigned long interval = 1000; 

    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();

        int elapsed = (lastUpdate - StateMachine::pairingStartTime) / 1000;
        
        Display::updateTimer(elapsed);
    }
}

static void updatePairing()
{
    updatePairingTimer();
    // HANDSHAKE CODE
    // DISPLAY.DRAWPAIRING TIMER CODE
}

static void pollLoRa()
{
    static unsigned long lastUpdate = 0;
    const unsigned long interval = 50;

    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();

        // uint8_t buffer[MAX_MESSAGE_LEN];

        // if (LoRa::isMessageAvailable())
        // {
        //     uint16_t size = LoRa::getMessage(buffer);

        //     if (size < LoRa::BUFFER_SIZE)
        //         buffer[size] = '\0';

        //     Serial.println((char*)buffer);

        //     Message msg;

        //     memcpy(msg.text, buffer, size);
        //     msg.text[size] = '\0';
        //     msg.outgoing = false;

        //     Messages::push(msg);

        //     // Display::showMessage((char*)buffer);
        // }
    }
}

static void enterState(DeviceState state)
{
    if (state == STATE_SLEEPING)
    {
        Display::drawSleepingScreen();
        Messages::clearInputBuffer();
        return;
    }

    Serial.println("ENTERING");
    Serial.println(state);
    Battery::wakeUp();
    // Display::drawActiveScreen(state);

    switch (state)
    {
        case STATE_CONNECTED:
            Display::drawConnectedScreen();
            Display::clearMessageLines();
            break;

        case STATE_PAIRING:
            Display::drawPairingScreen();
            enterPairing();
            break;
        
        case STATE_CONFIG:
            Display::drawConfigScreen();
            break;
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
        state = STATE_CONFIG;
    }
}

static void handleInput(const KeyEvent& event, DeviceState &state)
{
    switch (event.type)
    {
        case KeyEventType::CHARACTER:
            if (state == STATE_CONFIG && Messages::getInputLength() == 12) return;
            Messages::appendChar(event.character);
            break;

        case KeyEventType::BACKSPACE:
            Messages::backspace();
            break;

        case KeyEventType::ENTER:
            switch (state)
            {
                case STATE_CONNECTED:
                    sendMessage();
                    break;

                case STATE_CONFIG:
                    if (Messages::getInputLength() == 12) { state = STATE_PAIRING; }
                    break;
            }
            break;

        case KeyEventType::ESCAPE:
            switch (state)
            {
                case STATE_CONNECTED:
                    state = STATE_CONFIG;
                    break;

                case STATE_PAIRING:
                    state = STATE_CONFIG;
                    break;

                case STATE_CONFIG:
                    state = STATE_SLEEPING;
                    break;
            }
            break;
        default:
            break;
    }
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

    if (Messages::isInputDirty())
    {
        Display::updateConfigInputBuffer(Messages::getInputBuffer());
        Messages::clearInputDirty();
    }
}

static void runPairing(DeviceState &state) 
{
    KeyEvent event;

    if (Keyboard::getEvent(event))
    {
        handleInput(event, state);
    }

    updatePairing();

    // Update battery periodically (Every 3s)
    updateBattery();
}

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

    // Poll LoRa periodically ( Every 20ms )
    pollLoRa();

    Message msg;

    if (Messages::pop(msg))
    {
        Display::addMessage(msg);
        Display::drawMessages();
    }

    if (Messages::isInputDirty())
    {
        Display::updateMessageInputBuffer(Messages::getInputBuffer());
        Messages::clearInputDirty();
    }
}
