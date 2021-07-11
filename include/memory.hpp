#pragma once
#include <array>
#include <vector>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "APU/spc700.hpp"
#include "PPU/ppu.hpp"
#include "dma.hpp"
#include "cart.hpp"
#include "joypad.hpp"
#include "scheduler.hpp"
#include "math_engine.hpp"
#include "utils.hpp"

using json = nlohmann::json;

// For JIT-related reasons, all memory is made global and of course properly namespaced
// This helps avoid emitting member function calls, which can get finicky for a recompiler
namespace Memory {
    constexpr unsigned kilobyte = 1024;
    constexpr unsigned megabyte = 1024 * kilobyte;

    extern json gameDB; // Our game database containing info about each game's cart
    extern Cartridge cart; // Our game cartridge
    extern PPU* ppu;       // A pointer to the PPU
    extern Scheduler* scheduler; // A pointer to the scheduler
    extern MathEngine mathEngine; // A math engine that handles the multiplication/division ports and the M7 multiplication port
    extern DMAChannel dmaChannels[8]; // DMA channels
    extern SPC700 apu; // The audio processor

    // System memory
    extern std::array <u8, 128 * kilobyte> wram;

    // Software fastmem tables
    constexpr unsigned pageSize = 2048; // 2 Kilobyte pages
    constexpr unsigned pageCount = 0x1000000 / pageSize;

    static std::array <uint8_t*, pageCount> pageTableRead; // Page table for reads
    static std::array <uint8_t*, pageCount> pageTableWrite; // Page table for writes
    static u32 wramAddress = 0;

    static void loadROM (std::filesystem::path directory);
    static u8 read8 (u32 address); // Memory read handlers
    static u16 read16 (u32 address);

    static void write8 (u32 address, u8 value); // Memory write handlers
    static void write16 (u32 address, u16 value);

    template <bool isDebugger = false> // Slow memory handlers for stuff like IO, where fastmem doesn't work
    static u8 readSlow (u32 address); 

    template <bool isDebugger = false>
    static void writeSlow (u32 address, u8 value);

    template <bool isDebugger>
    void writeIO (u16 address, u8 value);
    
    static void writeIODMA (u16 address, u8 value) { writeIO <false> (address, value); } // Hack to make the linker happy

    static u8 read8Debugger (const u8* buffer, size_t address); // Frontend memory editor functions
    static void write8Debugger (u8* buffer, size_t address, u8 data);

    void mapFastmemPages();
}; // End Namespace Memory
