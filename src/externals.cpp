#include "snes.hpp"
#include "PPU/ppu.hpp"
#include "memory.hpp"
#include "dma.hpp"

// For JIT-related reasons, we have a crap ton of globals
// This file handles all extern declarations

// memory.hpp
// Cart info
json Memory::gameDB;
Cartridge Memory::cart;
PPU* Memory::ppu = nullptr;
DMAChannel Memory::dmaChannels[8];

// Memory areas
std::array <u8, 128 * Memory::kilobyte> Memory::wram;


// snes.hpp
SNES g_snes = SNES();