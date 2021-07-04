#pragma once
#include <array>
#include "BitField.hpp"
#include "utils.hpp"

union OAMAddr {
    u16 raw = 0;
    
    BitField <0, 8, u16> low;
    BitField <8, 8, u16> high;
};

union BGMode {
    u8 raw = 0;

    BitField <0, 3, u16> mode;
    BitField <3, 1, u16> bg3prio;
};

struct PPU {
    OAMAddr oamaddr;
    BGMode bgmode;

    std::array <u8, 200 * 200 * 4> framebuffer; // TODO: Actual coords
};