#pragma once
#include <stdint.h>

struct Message
{
    bool outgoing;
    char text[128];
};