#include "CPU/cpu.hpp"

void CPU::reset() {
    a.raw = 0;
    x = 0;
    y = 0;

    emulationMode = true;
    pb = 0;
    db = 0;

    psw.raw = 0x34; // Accumulator and index registers set to 8 bits, interrupts disabled
    sp = 0x1FC; // Initial SP
    pc = Memory::cart.resetVector;
}

void CPU::step() {
    printf ("hello");
}