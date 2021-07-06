#pragma once
#include <array>
#include "BitField.hpp"
#include "utils.hpp"

union OAMAddr {
    u16 raw = 0;
    
    BitField <0, 8, u16> low;
    BitField <8, 8, u16> high;
};

union VMAddr {
    u16 raw = 0;
    
    BitField <0, 8, u16> low;
    BitField <8, 8, u16> high;
};

union BGMode {
    u8 raw = 0;

    BitField <0, 3, u16> mode;
    BitField <3, 1, u16> bg3prio;
};

union VMain {
    u8 raw = 0;

    BitField <0, 2, u8> step; // (0..3 = Increment Word-Address by 1,32,128,128)
    BitField <2, 2, u8> translation; // (0..3 = 0bit/None, 8bit, 9bit, 10bit)
    BitField <7, 1, u8> incrementOnHigh; // Increment VRAM Address after accessing High/Low byte (0=Low, 1=High)
};

union BGSC {
    u8 raw = 0;

    BitField <2, 6, u8> base; // SC Base Address in VRAM (in 1K-word steps, aka 2K-byte steps)
    BitField <0, 2, u8> size; // SC Size (0=One-Screen, 1=V-Mirror, 2=H-Mirror, 3=Four-Screen)
};

enum class Depth {
    Bpp2, Bpp4, Bpp8
};

class PPU {
public:
    OAMAddr oamaddr;
    VMAddr vmaddr;
    BGMode bgmode;
    VMain vmain;

    BGSC sc[4]; // Screen Base and Screen Size registers
    uint8_t nba[4]; // BG Character Data Area Designation

    u8 rdnmi = 0x7F;
    u16 vramStep = 0; // Depending on vmain.step, this can be 1, 32 or 128

    std::array <u8, 256 * 224 * 4> framebuffer; // TODO: Actual coords
    std::array <u16, 0x8000> vram; // The VRAM. Note: This is 16-bit addressed, hence why the array is made of u16's. TODO: Put on heap?
    std::array <u16, 256> paletteRAM; // Palette RAM, addressed in words again
    
    u16 vofs[4] = { 0, 0, 0, 0 };
    u8 old_vofs[4] = { 0, 0, 0, 0 }; // Needed due to how VOFS writes work

    u16 hofs[4] = { 0, 0, 0, 0 };
    u16 old_hofs[4] = { 0, 0, 0, 0 }; // Needed due to how HOFS writes work

    u8 paletteAddr = 0; // The address in palette RAM we'll write to
    u8 latchedPalette = 0; // The LSB of a palette is latched and written on the second palette RAM write
    bool paletteLatch = false; // Tracks if this is the first or second palette RAM access

    int line = 0; // Line we're currently rendering

    PPU() {
        framebuffer.fill (0xFF);
    }

    void renderScanline();

    template <Depth depth, int number>
    void renderBG();
};