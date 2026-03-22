#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace Packet
{
    enum Type : uint8_t
    {
        HELLO = 1,
        HELLO_ACK = 2,
        AUTH = 3,
        AUTH_ACK = 4,
        DATA = 5
    };

    struct Packet
    {
        Type type;
        uint8_t length;
        uint8_t* payload;
    };

    bool parse(uint8_t* data, size_t len, Packet& outPkt);
}
