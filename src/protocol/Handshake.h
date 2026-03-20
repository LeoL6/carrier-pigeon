#pragma once
#include "Packet.h"

namespace Handshake
{
  void onHello(Packet::Packet& pkt);
  void onHelloAck(Packet::Packet& pkt);
  void onAuth(Packet::Packet& pkt);
  void onAuthAck(Packet::Packet& pkt);

  bool isEstablished();
}