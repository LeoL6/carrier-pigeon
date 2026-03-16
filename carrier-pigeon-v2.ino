#include "src/states/StateMachine.h"

DeviceState currentState = STATE_SLEEPING;

void setup()
{
    StateMachine::init();
}

void loop()
{
    StateMachine::update(currentState);
}
