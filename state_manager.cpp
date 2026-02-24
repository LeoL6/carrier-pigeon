#include "state_manager.h"

#include "states.h"

static DeviceState previousState = STATE_ACTIVE;

// Forward declarations
static void enterState(DeviceState state);
static void runActive(DeviceState &state);
static void runSleeping(DeviceState &state);

void state_manager::init()
{
    Keyboard::init();
    Display::init();
    LoRaManager::init();
    Battery::init();

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
        case STATE_ACTIVE:
            runActive(state);
            break;

        case STATE_SLEEPING:
            runSleeping(state);
            break;

        default:
            break;
    }
}