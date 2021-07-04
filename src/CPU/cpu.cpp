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
        case 0x1B: tcs(); break;
        case 0x5B: tcd(); break;
        case 0x8A: txa(); break;
        case 0x98: tya(); break;
        case 0x9A: txs(); break;
        case 0xA8: tay(); break;

        case 0xA0: ldy_imm(); break;
        case 0xA2: ldx_imm(); break;
        case 0xA9: lda_imm(); break;

        case 0x84: sty <AddressingModes::Direct>(); break;
        case 0x8C: sty <AddressingModes::Absolute>(); break;
        case 0x94: sty <AddressingModes::Direct_x>(); break;

        case 0x81: sta <AddressingModes::Direct_indirect_x>(); break;
        case 0x83: sta <AddressingModes::Stack_relative>(); break;
        case 0x85: sta <AddressingModes::Direct>(); break;
        case 0x87: sta <AddressingModes::Direct_indirect_long>(); break;
        case 0x8D: sta <AddressingModes::Absolute>(); break;
        case 0x8F: sta <AddressingModes::Absolute_long>(); break;
        case 0x91: sta <AddressingModes::Direct_indirect_y>(); break;
        case 0x92: sta <AddressingModes::Direct_indirect>(); break;
        case 0x93: sta <AddressingModes::Stack_relative_indirect_indexed>(); break;
        case 0x95: sta <AddressingModes::Direct_x>(); break;
        case 0x97: sta <AddressingModes::Direct_indirect_long_y>(); break;
        case 0x99: sta <AddressingModes::Absolute_y>(); break;
        case 0x9D: sta <AddressingModes::Absolute_x>(); break;
        case 0x9F: sta <AddressingModes::Absolute_long_x>(); break;

        case 0x8E: stx <AddressingModes::Absolute>(); break;
        case 0x9C: stz <AddressingModes::Absolute>(); break;

        case 0x88: dey(); break;
        case 0xCA: dex(); break;
        case 0xC8: iny(); break;
        case 0xE8: inx(); break;

        case 0x08: php(); break;
        case 0x4B: phk(); break;
        case 0xAB: plb(); break;

        case 0x2C: bit <AddressingModes::Absolute>(); break;

        case 0x10: relativeJump (!psw.sign); break; // bmi
        case 0x30: relativeJump (psw.sign); break; // bmi
        case 0x50: relativeJump (!psw.overflow); break; // bvc
        case 0x70: relativeJump (psw.overflow); break; // bvs
        case 0x80: relativeJump (true); break; // bra
        case 0xD0: relativeJump (!psw.zero); break; // bne
        case 0xF0: relativeJump (psw.zero); break; // beq
        
        case 0x4C: jmp <AddressingModes::Absolute>(); break;
        case 0x5C: jmp <AddressingModes::Absolute_long>(); break;
        case 0x7C: jmp <AddressingModes::Absolute_indirect_x>(); break;
        case 0xDC: jmp <AddressingModes::Absolute_indirect_long>(); break;

        case 0x20: jsr <AddressingModes::Absolute>(); break;
        case 0x22: jsr <AddressingModes::Absolute_long>(); break;
        case 0xFC: jsr <AddressingModes::Absolute_indirect_x>(); break;

        case 0xC1: cmp <AddressingModes::Direct_indirect_x>(); break;
        case 0xC3: cmp <AddressingModes::Stack_relative>(); break;
        case 0xC5: cmp <AddressingModes::Direct>(); break;
        case 0xC7: cmp <AddressingModes::Direct_indirect_long>(); break;
        case 0xC9: cmp_imm(); break;
        case 0xCD: cmp <AddressingModes::Absolute>(); break;
        case 0xCF: cmp <AddressingModes::Absolute_long>(); break;
        case 0xD1: cmp <AddressingModes::Direct_indirect_y>(); break;
        case 0xD2: cmp <AddressingModes::Direct_indirect>(); break;
        case 0xD3: cmp <AddressingModes::Stack_relative_indirect_indexed>(); break;
        case 0xD5: cmp <AddressingModes::Direct_x>(); break;
        case 0xD7: cmp <AddressingModes::Direct_indirect_long_y>(); break;
        case 0xD9: cmp <AddressingModes::Absolute_y>(); break;
        case 0xDD: cmp <AddressingModes::Absolute_x>(); break;
        case 0xDF: cmp <AddressingModes::Absolute_long_x>(); break;

        case 0xE9: sbc_imm(); break;

        case 0x18: clc(); break;
        case 0x38: sec(); break;
        case 0x78: sei(); break;
        case 0xC2: rep(); break;
        case 0xE2: sep(); break;
        case 0xFB: xce(); break;

        case 0xEA: cycles = 2; break; // NOP

        default: Helpers::panic ("Unknown opcode: {:02X}\n", opcode);
    }
}