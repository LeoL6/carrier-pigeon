#pragma once
#include <stdint.h>

#define MAX_MESSAGE_LEN 64

struct Message
{
    char text[MAX_MESSAGE_LEN];
    bool outgoing;
};