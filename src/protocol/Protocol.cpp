#include "Protocol.h"
#include "Handshake.h"

namespace Protocol
{
  // <========================>
  //   Callback
  // <========================>
  static DataCallback onDataCb = nullptr;

  void setDataCallback(DataCallback cb)
  {
    onDataCb = cb;
  }
  
  // <========================>
  //   Packet Rx Function
  // <========================>
  void onReceive(Packet::Packet& pkt)
  {
    switch (pkt.type)
    {
      case Packet::HELLO:
        Serial.println("RECEIVED HELLO");
        Handshake::onHello(pkt);
        break;

      case Packet::HELLO_ACK:
        Serial.println("RECEIVED HELLO ACK");
        Handshake::onHelloAck(pkt);
        break;

      case Packet::AUTH:
        Serial.println("RECEIVED AUTH");
        Handshake::onAuth(pkt);
        break;

      case Packet::AUTH_ACK:
        Serial.println("RECEIVED AUTH ACK");
        Handshake::onAuthAck(pkt);
        break;

      case Packet::DATA:
        if (!Handshake::isEstablished()) return;

        // Serial.println((char*)pkt.payload);

        if (onDataCb)
          onDataCb(pkt);

        // Secure::onData(pkt);
        break;

      default:
        // ignore unknown packet types
        break;
    }
  }
}