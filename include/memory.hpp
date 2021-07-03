#pragma once
#include <array>
#include <vector>
#include <filesystem>
#include "cart.hpp"
#include "nlohmann/json.hpp"
#include "utils.hpp"

using json = nlohmann::json;

// For JIT-related reasons, all memory is made global and of course properly namespaced
// This helps avoid emitting member function calls, which can get finicky for a recompiler
namespace Memory {
    constexpr unsigned kilobyte = 1024;
    constexpr unsigned megabyte = 1024 * kilobyte;

    extern json gameDB; // Our game database containing info about each game's cart
    extern Cartridge cart;

    // System memory
    extern std::array <u8, 128 * kilobyte> wram;

    // Software fastmem tables
    constexpr unsigned pageSize = 2048; // 2 Kilobyte pages
    constexpr unsigned pageCount = 0x1000000 / pageSize;

    static std::array <uint8_t*, pageCount> pageTableRead; // Page table for reads
    static std::array <uint8_t*, pageCount> pageTableWrite; // Page table for writes

    void loadROM (std::filesystem::path directory);
    u8 read8 (u32 address);
    u16 read16 (u32 address);

    void write8 (u32 address, u8 value);
    void write16 (u32 address, u16 value);
    
    void mapFastmemPages();
}; // End Namespace Memory