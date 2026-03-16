#pragma once

#include "../states/States.h"
#include "../message/Message.h"

namespace Display {
    // <=====================>
    // Setup Functions
    // <=====================>
    void init();

    // <=====================>
    // Full Screen Functions
    // <=====================>
    void drawActiveScreen(DeviceState &state);
    void drawSleepingScreen();

    // <=====================>
    // Partial Screen Functions
    // <=====================>
    void updateBattery(int percentage, bool charging);
    void updateInputBuffer(const char* keyBuffer);

    // <=====================>
    // Message Functions
    // <=====================>
    void addMessage(const Message& msg);
    void drawMessages();
    void clearMessageLines();
}

