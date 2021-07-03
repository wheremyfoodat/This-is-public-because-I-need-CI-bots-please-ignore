#pragma once
#include <filesystem>
#include "utils.hpp"
#include "CPU/cpu.hpp"
#include "memory.hpp"

class SNES {
public:
    SNES();
    void step();
    void runFrame();
    void reset();

    CPU cpu;
}; // End Namespace SNES

extern SNES g_snes; // a global SNES object