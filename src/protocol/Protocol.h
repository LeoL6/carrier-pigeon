#pragma once
#include "Packet.h"
#define DEBUG_AIRTIME

namespace Protocol
{  
  // <========================>
  //   Callback
  // <========================>
  using DataCallback = void(*)(const uint8_t* decrypted, size_t len);
  void setDataCallback(DataCallback cb);

  // <========================>
  //   Packet Functions
  // <========================>
  void onReceive(Packet::Packet& pkt);

  // <========================>
  //   Data Functions
  // <========================>
  void sendData(const uint8_t* data, size_t len);
}