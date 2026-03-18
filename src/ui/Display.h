#pragma once

#include "../states/States.h"
#include "../message/Message.h"

namespace Display {
    // <=====================>
    //   Setup Functions
    // <=====================>
    void init();

    // <=====================>
    //   Full Screen Functions
    // <=====================>
    void drawActiveScreen(DeviceState &state);
    void drawSleepingScreen();

    // <=====================>
    //   Partial Screen Functions
    // <=====================>
    void updateBattery(int percentage, bool charging);
    void updateInputBuffer(const char* keyBuffer);
    void updateTimer(int elapsed);

    // <=====================>
    //   Message Functions
    // <=====================>
    void addMessage(const Message& msg);
    void drawMessages();
    void clearMessageLines();

    // <=====================>
    //   Constants
    // <=====================>
    static constexpr unsigned int SCREEN_WIDTH  = 416;
    static constexpr unsigned int SCREEN_HEIGHT = 240;
}

