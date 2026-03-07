#pragma once

namespace Battery
{
  void wakeUp();
  bool isUsbConnected();
  int readPercentage();
}