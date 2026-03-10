#pragma once

enum class PacketType : uint8_t
{
    CHAT,
    HELLO,
    KEY,
    CONFIRM
};

struct Packet
{
    uint8_t type;
    uint8_t length;
    uint8_t payload[220];
};
