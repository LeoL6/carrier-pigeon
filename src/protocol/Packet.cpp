#include "Packet.h"

namespace Packet
{
  bool parse(uint8_t* data, size_t len, Packet& outPkt)
  {
    // Packet Length Validation
    if (len < 2) return false;

    uint8_t type = data[0];
    uint8_t payloadLen = data[1];

    // Packet Type Validation
    if (type < HELLO || type > DATA) return false;

    if (payloadLen != len - 2) return false;

    outPkt.type = (Type)type;
    outPkt.length = payloadLen;
    outPkt.payload = &data[2];

    return true;
  }
}