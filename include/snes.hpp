#pragma once
#include "utils.hpp"
#include "CPU/cpu.hpp"
#include "memory.hpp"

class SNES {
public:
    void step();
    void runFrame();

    CPU cpu;
}; // End Namespace SNES

static auto g_snes = SNES(); // a global SNES object