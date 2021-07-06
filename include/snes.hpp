#pragma once
#include <filesystem>
#include "utils.hpp"
#include "CPU/cpu.hpp"
#include "PPU/ppu.hpp"
#include "memory.hpp"
#include "joypad.hpp"
#include "scheduler.hpp"

class SNES {
public:
    SNES();
    void step();
    void runFrame();
    void reset();

    CPU cpu;
    PPU ppu;
    Scheduler scheduler;
    bool frameDone = false; // Can we render and go back to the GUI now?
}; // End Namespace SNES

extern SNES g_snes; // a global SNES object