#pragma once

#include "KeyEvent.h"

namespace Keyboard {
  void init();
  bool getEvent(KeyEvent& event);
}