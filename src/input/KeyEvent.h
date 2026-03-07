#pragma once

enum class KeyEventType
{
    CHARACTER,
    ENTER,
    BACKSPACE,
    ESCAPE
    // ARROW_UP,
    // ARROW_DOWN,
    // ARROW_LEFT,
    // ARROW_RIGHT
};

struct KeyEvent
{
    KeyEventType type;
    char character;   // Only valid if type == CHARACTER
};