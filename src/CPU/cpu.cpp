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

        case 0xA1: lda <AddressingModes::Direct_indirect_x>(); break;
        case 0xA3: lda <AddressingModes::Stack_relative>(); break;
        case 0xA5: lda <AddressingModes::Direct>(); break;
        case 0xA7: lda <AddressingModes::Direct_indirect_long>(); break;
        case 0xA9: lda_imm(); break;
        case 0xAD: lda <AddressingModes::Absolute>(); break;
        case 0xAF: lda <AddressingModes::Absolute_long>(); break;
        case 0xB1: lda <AddressingModes::Direct_indirect_y>(); break;
        case 0xB2: lda <AddressingModes::Direct_indirect>(); break;
        case 0xB3: lda <AddressingModes::Stack_relative_indirect_indexed>(); break;
        case 0xB5: lda <AddressingModes::Direct_x>(); break;
        case 0xB7: lda <AddressingModes::Direct_indirect_long_y>(); break;
        case 0xB9: lda <AddressingModes::Absolute_y>(); break;
        case 0xBD: lda <AddressingModes::Absolute_x>(); break;
        case 0xBF: lda <AddressingModes::Absolute_long_x>(); break;

        case 0xA0: ldy_imm(); break;
        case 0xA2: ldx_imm(); break;

        case 0xA6: ldx <AddressingModes::Direct>(); break;

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

        case 0x86: stx <AddressingModes::Direct>(); break;
        case 0x8E: stx <AddressingModes::Absolute>(); break;
        case 0x9C: stz <AddressingModes::Absolute>(); break;

        case 0x4A: lsr_accumulator(); break;

        case 0x88: dey(); break;
        case 0xCA: dex(); break;
        case 0xC8: iny(); break;
        case 0xE8: inx(); break;

        case 0x08: php(); break;
        case 0x0B: phd(); break;
        case 0x28: plp(); break;
        case 0x2B: pld(); break;
        case 0x4B: phk(); break;
        case 0x48: pha(); break;
        case 0x5A: phy(); break;
        case 0x62: per(); break;
        case 0x68: pla(); break;
        case 0x7A: ply(); break;
        case 0x8B: phb(); break;
        case 0xDA: phx(); break;
        case 0xAB: plb(); break;
        case 0xD4: pei(); break;
        case 0xF4: pea(); break;
        case 0xFA: plx(); break;

        case 0x24: bit <AddressingModes::Direct>(); break;
        case 0x2C: bit <AddressingModes::Absolute>(); break;
        case 0x34: bit <AddressingModes::Direct_x>(); break;
        case 0x3C: bit <AddressingModes::Absolute_x>(); break;

        case 0x10: relativeJump (!psw.sign); break; // bmi
        case 0x30: relativeJump (psw.sign); break; // bmi
        case 0x50: relativeJump (!psw.overflow); break; // bvc
        case 0x70: relativeJump (psw.overflow); break; // bvs
        case 0x80: relativeJump (true); break; // bra
        case 0x82: brl(); break;
        case 0x90: relativeJump (!psw.carry); break; // bcc
        case 0xB0: relativeJump (psw.carry); break; // bcs
        case 0xD0: relativeJump (!psw.zero); break; // bne
        case 0xF0: relativeJump (psw.zero); break; // beq
        
        case 0x4C: jmp <AddressingModes::Absolute>(); break;
        case 0x5C: jmp <AddressingModes::Absolute_long>(); break;
        case 0x6C: jmp <AddressingModes::Absolute_indirect>(); break;
        case 0x7C: jmp <AddressingModes::Absolute_indirect_x>(); break;
        case 0xDC: jmp <AddressingModes::Absolute_indirect_long>(); break;

        case 0x20: jsr <AddressingModes::Absolute>(); break;
        case 0x22: jsr <AddressingModes::Absolute_long>(); break;
        case 0xFC: jsr <AddressingModes::Absolute_indirect_x>(); break;

        case 0x60: rts(); break;
        case 0x6B: rtl(); break;

        case 0x01: ora <AddressingModes::Direct_indirect_x>(); break;
        case 0x03: ora <AddressingModes::Stack_relative>(); break;
        case 0x05: ora <AddressingModes::Direct>(); break;
        case 0x07: ora <AddressingModes::Direct_indirect_long>(); break;
        case 0x09: ora_imm(); break;
        case 0x0D: ora <AddressingModes::Absolute>(); break;
        case 0x0F: ora <AddressingModes::Absolute_long>(); break;
        case 0x11: ora <AddressingModes::Direct_indirect_y>(); break;
        case 0x12: ora <AddressingModes::Direct_indirect>(); break;
        case 0x13: ora <AddressingModes::Stack_relative_indirect_indexed>(); break;
        case 0x15: ora <AddressingModes::Direct_x>(); break;
        case 0x17: ora <AddressingModes::Direct_indirect_long_y>(); break;
        case 0x19: ora <AddressingModes::Absolute_y>(); break;
        case 0x1D: ora <AddressingModes::Absolute_x>(); break;
        case 0x1F: ora <AddressingModes::Absolute_long_x>(); break;

        case 0x21: and_ <AddressingModes::Direct_indirect_x>(); break;
        case 0x23: and_ <AddressingModes::Stack_relative>(); break;
        case 0x25: and_ <AddressingModes::Direct>(); break;
        case 0x27: and_ <AddressingModes::Direct_indirect_long>(); break;
        case 0x29: and_imm(); break;
        case 0x2D: and_ <AddressingModes::Absolute>(); break;
        case 0x2F: and_ <AddressingModes::Absolute_long>(); break;
        case 0x31: and_ <AddressingModes::Direct_indirect_y>(); break;
        case 0x32: and_ <AddressingModes::Direct_indirect>(); break;
        case 0x33: and_ <AddressingModes::Stack_relative_indirect_indexed>(); break;
        case 0x35: and_ <AddressingModes::Direct_x>(); break;
        case 0x37: and_ <AddressingModes::Direct_indirect_long_y>(); break;
        case 0x39: and_ <AddressingModes::Absolute_y>(); break;
        case 0x3D: and_ <AddressingModes::Absolute_x>(); break;
        case 0x3F: and_ <AddressingModes::Absolute_long_x>(); break;

        case 0x41: eor <AddressingModes::Direct_indirect_x>(); break;
        case 0x43: eor <AddressingModes::Stack_relative>(); break;
        case 0x45: eor <AddressingModes::Direct>(); break;
        case 0x47: eor <AddressingModes::Direct_indirect_long>(); break;
        case 0x49: eor_imm(); break;
        case 0x4D: eor <AddressingModes::Absolute>(); break;
        case 0x4F: eor <AddressingModes::Absolute_long>(); break;
        case 0x51: eor <AddressingModes::Direct_indirect_y>(); break;
        case 0x52: eor <AddressingModes::Direct_indirect>(); break;
        case 0x53: eor <AddressingModes::Stack_relative_indirect_indexed>(); break;
        case 0x55: eor <AddressingModes::Direct_x>(); break;
        case 0x57: eor <AddressingModes::Direct_indirect_long_y>(); break;
        case 0x59: eor <AddressingModes::Absolute_y>(); break;
        case 0x5D: eor <AddressingModes::Absolute_x>(); break;
        case 0x5F: eor <AddressingModes::Absolute_long_x>(); break;

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

        case 0xC0: cpy_imm(); break;
        case 0xC4: cpy <AddressingModes::Direct>(); break;
        case 0xCC: cpy <AddressingModes::Absolute>(); break;
        case 0xE0: cpx_imm(); break;
        case 0xE4: cpx <AddressingModes::Direct>(); break;
        case 0xEC: cpx <AddressingModes::Absolute>(); break;

        case 0x69: adc_imm(); break;
        case 0xE9: sbc_imm(); break;

        case 0x18: clc(); break;
        case 0x38: sec(); break;
        case 0x58: cli(); break;
        case 0x78: sei(); break;
        case 0xB8: clv(); break;
        case 0xC2: rep(); break;
        case 0xD8: cld(); break;
        case 0xE2: sep(); break;
        case 0xF8: sed(); break;
        case 0xFB: xce(); break;

        case 0x00: brk(); break;
        case 0x02: cop(); break;
        case 0x40: rti(); break;
        case 0x42: wdm(); break;
        case 0xCB: wai(); break;
        case 0xDB: stp(); break;

        case 0xEA: cycles = 2; break; // NOP

        default: Helpers::panic ("Unknown opcode: {:02X}\n", opcode);
    }
}