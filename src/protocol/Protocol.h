#pragma once
#include "Packet.h"

namespace Protocol
{
  // <========================>
  //   Callback
  // <========================>
  using DataCallback = void(*)(const Packet::Packet&);
  void setDataCallback(DataCallback cb);

  // <========================>
  //   Packet Functions
  // <========================>
  void onReceive(Packet::Packet& pkt);
}