#include "memory.hpp"

// For JIT-related reasons, we have a crap ton of globals
// This file handles all extern declarations

// Memory.hpp
// Cart info
unsigned Memory::romSize = 0;
Mappers Memory::mapper = Mappers::NoCart;

// Exception vector
u16 Memory::resetVector = 0;
u16 Memory::nmiVector = 0;
u16 Memory::irqVector = 0;
u16 Memory::copVector = 0;
u16 Memory::brkVector = 0;