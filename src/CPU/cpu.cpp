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
        case 0x5B: tcd(); break;
        case 0x9A: txs(); break;

        case 0xA2: ldx_imm(); break;
        case 0xA9: lda_imm(); break;

        case 0x8D: sta <AddressingModes::Absolute>(); break;
        case 0x9C: stz <AddressingModes::Absolute>(); break;

        case 0x4B: phk(); break;
        case 0xAB: plb(); break;

        case 0x18: clc(); break;
        case 0x78: sei(); break;
        case 0xC2: rep(); break;
        case 0xE2: sep(); break;
        case 0xFB: xce(); break;
        default: Helpers::panic ("Unknown opcode: {:02X}\n", opcode);
    }
}