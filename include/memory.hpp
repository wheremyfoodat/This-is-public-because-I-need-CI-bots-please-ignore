#pragma once
#include <array>
#include <vector>
#include "utils.hpp"

enum class Mappers {
    NoCart,         // There is no cart inserted, pls add one
    LoROM_Standard, // LoROM without coprocs, up to 2MB (Eg: Super Mario World)
    LoROM_Big       // LoROM without coprocs, over 2MB, some banks stored in HiROM area (Eg: Super Metroid)
};

// For JIT-related reasons, all memory is made global and of course properly namespaced
// This helps avoid emitting member function calls, which can get finicky for a recompiler
namespace Memory {
    constexpr unsigned kilobyte = 1024;
    constexpr unsigned megabyte = 1024 * kilobyte;

    static std::array <u8, 32 * megabyte> codeBuffer; // This is where the generated recompiler code goes

    // System memory
    static std::array <u8, 128 * kilobyte> wram; // All memory is declared as static arrays
    static std::array <u8, 4 * megabyte> rom; // This allows for it to be accessed using pc-relative accesses in the emitted JIT code
    extern unsigned romSize;
    extern Mappers mapper;

    // Exception vectors
    extern u16 resetVector;
    extern u16 nmiVector;
    extern u16 irqVector;
    extern u16 copVector;
    extern u16 brkVector;

    void loadROM (std::filesystem::path directory);

    std::string getMapperName (Mappers mapper);
    Mappers getMapperType();
}; // End Namespace Memory