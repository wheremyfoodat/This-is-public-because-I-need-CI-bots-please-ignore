#pragma once
#include <filesystem>
#include <condition_variable>
#include <mutex>
#include <atomic>
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

    void runAsync();
    void waitSignal(); 

    CPU cpu;
    PPU ppu;
    Scheduler scheduler;
    bool frameDone = true; // Can we render and go back to the GUI now?
    
    std::condition_variable emu_condition_variable;
    std::mutex emu_mutex;
    std::atomic <bool> run_emu_thread = false;
}; // End Namespace SNES

extern SNES g_snes; // a global SNES object