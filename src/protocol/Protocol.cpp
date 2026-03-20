#include "Handshake.h"

namespace Protocol
{
  void onReceive(Packet::Packet& pkt)
  {
    switch (pkt.type)
    {
      case Packet::HELLO:
        Handshake::onHello(pkt);
        break;

      case Packet::HELLO_ACK:
        Handshake::onHelloAck(pkt);
        break;

      case Packet::AUTH:
        Handshake::onAuth(pkt);
        break;

      case Packet::AUTH_ACK:
        Handshake::onAuthAck(pkt);
        break;

      case Packet::DATA:
        if (!Handshake::isEstablished()) return;

        // Secure::onData(pkt);
        break;

      default:
        // ignore unknown packet types
        break;
    }
  }
}