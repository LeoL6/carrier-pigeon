#pragma once

namespace Display {
    // <=====================>
    // Setup Functions
    // <=====================>
    void init();

    // <=====================>
    // Full Screen Functions
    // <=====================>
    void drawActiveScreen();
    void drawSleepingScreen();

    // <=====================>
    // Partial Screen Functions
    // <=====================>
    void updateBattery(int percentage, bool charging);
    void updateInputBuffer(const char* keyBuffer);
}

