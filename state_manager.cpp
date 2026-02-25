#include "state_manager.h"

#include "states.h"

static DeviceState previousState = STATE_SLEEPING;

// Forward declarations
static void enterState(DeviceState state);
static void runActive(DeviceState &state);
static void runSleeping(DeviceState &state);


static char keyBuffer[64];
static int bufferIndex = 0;

void state_manager::init()
{
    keyboard_manager::init();
    display_manager::init();
    lora_manager::init();
    battery_manager::init();

    enterState(STATE_ACTIVE);
}

void state_manager::update(DeviceState &state)
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
            break;

        case STATE_CONNECTING:
            break;

        case STATE_CONNECTED:
            runActive(state);
            break;

        default:
            break;
    }
}

static void enterState(DeviceState state)
{
    switch (state)
    {
        case STATE_ACTIVE:
            display_manager::drawActiveScreen();
            break;

        case STATE_SLEEPING:
            display_manager::drawSleepScreen();
            break;

        default:
            break;
    }
}

// void handleChar(char c) {
//   switch (c) {
//     case 13:
//       if (bufferIndex > 0) {
//         Serial.println("Enter");
//       }
//       break;

//     case 8: 
//       if (bufferIndex > 0) {
//         bufferIndex--;
//         keyBuffer[bufferIndex] = '\0';
//       }
//       break;

//     case 27:
//       //do turn off stuff
//       Serial.println("Hibernate");
//       hibernate();
//       return;

//     case 180:
//       Serial.println("LEFT");
//       break;

//     case 181:
//       Serial.println("UP");
//       break;

//     case 182:
//       Serial.println("DOWN");
//       break;

//     case 183:
//       Serial.println("RIGHT");
//       break;

//     default:
//       keyBuffer[bufferIndex++] = (char)c;
//       keyBuffer[bufferIndex] = '\0';
//       break;
//   }
// }

static void runActive(DeviceState &state)
{
    keyboard_manager::update();
    lora_manager::update();

    // Handle keyboard input
    if (keyboard_manager::hasKey())
    {
        char key = keyboard_manager::getKey();

        switch (key)
        {
            case 27: // ESC
                state = STATE_SLEEPING;
                return;

            case '\n': // Enter
                lora_manager::sendMessage(keyBuffer);
                bufferIndex = 0;
                keyBuffer[0] = '\0';
                break;

            case 8: // Backspace
                if (bufferIndex > 0)
                {
                    bufferIndex--;
                    keyBuffer[bufferIndex] = '\0';
                }
                break;

            default:
                if (bufferIndex < sizeof(keyBuffer) - 1)
                {
                    keyBuffer[bufferIndex++] = key;
                    keyBuffer[bufferIndex] = '\0';
                }
                break;
        }
    }

    // Update battery periodically
    float voltage = Battery::readVoltage();
    display_manager::updateBattery(voltage);

    // Update UI with buffer content
    display_manager::updateInputBuffer(keyBuffer);
}

static void runSleeping(DeviceState &state)
{
    keyboard_manager::update();

    if (keyboard_manager::hasKey())
    {
        // Wake on any key
        // state = STATE_CONFIG;
        state = STATE_CONNECTING;
    }

    // esp_sleep_enable_timer_wakeup(20000); // 20ms
    // esp_light_sleep_start();
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
