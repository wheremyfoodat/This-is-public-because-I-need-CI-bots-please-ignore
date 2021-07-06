#pragma once
#include <filesystem>
#include "utils.hpp"
#include "CPU/cpu.hpp"
#include "PPU/ppu.hpp"
#include "memory.hpp"
#include "joypad.hpp"

class SNES {
public:
    SNES();
    void step();
    void runFrame();
    void reset();

    CPU cpu;
    PPU ppu;
}; // End Namespace SNES

extern SNES g_snes; // a global SNES object