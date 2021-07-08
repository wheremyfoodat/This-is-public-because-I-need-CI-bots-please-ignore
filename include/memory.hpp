#pragma once
#include <array>
#include <vector>
#include <filesystem>
#include "nlohmann/json.hpp"
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
    extern Cartridge cart;
    extern PPU* ppu;
    extern Scheduler* scheduler;
    extern MathEngine mathEngine;
    extern DMAChannel dmaChannels[8];

    // System memory
    extern std::array <u8, 128 * kilobyte> wram;

    // Software fastmem tables
    constexpr unsigned pageSize = 2048; // 2 Kilobyte pages
    constexpr unsigned pageCount = 0x1000000 / pageSize;

    static std::array <uint8_t*, pageCount> pageTableRead; // Page table for reads
    static std::array <uint8_t*, pageCount> pageTableWrite; // Page table for writes
    static u32 wramAddress = 0;

    void loadROM (std::filesystem::path directory);
    u8 read8 (u32 address); // Memory read handlers
    u16 read16 (u32 address);

    void write8 (u32 address, u8 value); // Memory write handlers
    void write16 (u32 address, u16 value);

    template <bool isDebugger = false> // Slow memory handlers for stuff like IO, where fastmem doesn't work
    u8 readSlow (u32 address); 

    template <bool isDebugger = false>
    void writeSlow (u32 address, u8 value);

    template <bool isDebugger>
    void writeIO (u16 address, u8 value);
    
    static void writeIODMA (u16 address, u8 value) { writeIO <false> (address, value); } // Hack to make the linker happy

    u8 read8Debugger (const u8* buffer, size_t address); // Frontend memory editor functions
    void write8Debugger (u8* buffer, size_t address, u8 data);

    void mapFastmemPages();
}; // End Namespace Memory