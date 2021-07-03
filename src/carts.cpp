#include <cassert>
#include <filesystem>
#include "utils.hpp"
#include "memory.hpp"

static void Memory::loadROM (std::filesystem::path directory) {
    if (directory.empty()) // Don't do anything if the directory is empty
        return; 

    rom.fill(0);
    const auto file = Helpers::loadROM (directory.string());
    assert (file.size() <= 4 * Memory::megabyte); // All valid SNES carts should be <= 8MB?

    Memory::romSize = file.size(); // Set the ROM size to the size of the given file
    for (auto i = 0; i < file.size(); i++) // Copy ROM to the ROM array which is in static storage to be accessible via pc-relative accesses
        Memory::rom[i] = file[i];

    Memory::mapper = getMapperType(); // Initialize exception vectors and mapper info
}

// Use heuristics and the ROM size to find the mapper type
static Mappers Memory::getMapperType() {
    u16 entrypoint = (rom[0x7FFD] << 8) | rom[0x7FFC]; // Assume the ROM is LoROM, then use heuristics to make sure

    if (Memory::rom[entrypoint - 0x8000] == 0x78) { // Check that the first byte is a di opcode, otherwise it's probably not an entrypoint
        for (auto i = 1; i <= 64; i++) { // Look for `clc; xce` in the first 64 bytes after
            if (Memory::rom[entrypoint + i - 0x8000] == 0x18 && Memory::rom[entrypoint + i + 1 - 0x8000] == 0xFB) {
                Memory::resetVector = entrypoint; // Fetch exception vectors
                Memory::copVector = (rom[0x7FE5] << 8) | rom[0x7FE4];
                Memory::brkVector = (rom[0x7FE7] << 8) | rom[0x7FE6];
                Memory::nmiVector = (rom[0x7FEB] << 8) | rom[0x7FEA];
                Memory::irqVector = (rom[0x7FEF] << 8) | rom[0x7FEE];

                if (Memory::romSize >= 2 * Memory::megabyte)
                    return Mappers::LoROM_Big;
                else
                    return Mappers::LoROM_Standard;
            }
        }
    }

    Helpers::panic ("Couldn't identify mapper :(\n");
}

static std::string Memory::getMapperName (Mappers mapper) {
    switch (mapper) {
        case Mappers::NoCart: return "No cartridge inserted";
        case Mappers::LoROM_Standard: return "Standard LoROM";
        case Mappers::LoROM_Big: return "Big LoROM";
        
        default: return "Unknown";
    }
}