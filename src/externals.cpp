#include "snes.hpp"
#include "scheduler.hpp"
#include "APU/spc700.hpp"
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
Scheduler* Memory::scheduler = nullptr;
MathEngine Memory::mathEngine;
DMAChannel Memory::dmaChannels[8];
SPC700 Memory::apu;

// Memory areas
std::array <u8, 128 * Memory::kilobyte> Memory::wram;

// Joypad.hpp
u16 Joypads::pad1 = 0;

// snes.hpp
SNES g_snes = SNES();