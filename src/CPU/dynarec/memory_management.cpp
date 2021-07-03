#include "CPU/cpu.hpp"
#include "utils.hpp"

// Initializes the page tables for the recompiler
void CPU::initMemory() {
    recompilerBankLUT.fill (nullptr); // Mark all pages as unmapped
    
    // It might seem insane to allocate 256MB for this and it is
    // Gotta find a less naive memory allocation scheme later on
    // We have 1 pointer for each address, * 8 because of the 3 PSW bits that are part of the checksum
    recompilerROM = new uintptr_t[4 * Memory::megabyte * 8](); 
    recompilerWRAM = new uintptr_t[128 * Memory::kilobyte * 8]();

    // Map system area RAM...
    // for (auto pb = 0; pb <= 0x3F; pb++) {

    // }
}