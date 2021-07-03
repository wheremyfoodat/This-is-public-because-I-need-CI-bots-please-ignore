#include "snes.hpp"
#include "memory.hpp"

// For JIT-related reasons, we have a crap ton of globals
// This file handles all extern declarations

// memory.hpp
// Cart info
json Memory::gameDB;
Cartridge Memory::cart;

// Memory areas
std::array <u8, 128 * Memory::kilobyte> Memory::wram;

// snes.hpp
SNES g_snes = SNES();