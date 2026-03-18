#pragma once
#include "States.h"

namespace StateMachine {
    void init();
    void update(DeviceState &state);

    static unsigned long pairingStartTime = 0;
}