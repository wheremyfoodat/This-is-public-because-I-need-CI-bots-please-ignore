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
    //uintptr_t pageTableRead

    void loadROM (std::filesystem::path directory);
}; // End Namespace Memory