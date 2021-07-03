#include "CPU/cpu.hpp"

void CPU::reset() {
    a.raw = 0;
    x = 0;
    y = 0;

    emulationMode = true;
    setPB (0);
    setDB (0);

    psw.raw = 0x34; // Accumulator and index registers set to 8 bits, interrupts disabled
    sp = 0x1FC; // Initial SP
    pc = Memory::cart.resetVector; // Set PC to the reset vector in the cartridge
}

void CPU::step() {
    const auto opcode = nextByte();
    executeOpcode (opcode);
}

void CPU::executeOpcode (u8 opcode) {
    switch (opcode) {
        case 0x78: psw.irqDisable = true; break; // SEI
        default: Helpers::panic ("Unknown opcode: {:02X}\n", opcode);
    }
}