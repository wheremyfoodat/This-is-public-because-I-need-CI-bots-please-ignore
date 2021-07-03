#pragma once
#include "BitField.hpp"
#include "utils.hpp"

struct PPU {
    union {
        u16 raw = 0;
        BitField <0, 8, u16> low;
        BitField <8, 8, u16> high;
    } oamaddr;
};