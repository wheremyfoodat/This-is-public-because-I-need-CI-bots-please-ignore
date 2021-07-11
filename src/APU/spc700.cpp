#include "APU/spc700.hpp"

// Run 1 SPC700 opcode
void SPC700::executeOpcode() {
    const auto opcode = nextByte();
    cycles += cycleTable[opcode];

    switch (opcode) {
        case 0x02: set <0>(); break;
        case 0x22: set <1>(); break;
        case 0x42: set <2>(); break;
        case 0x62: set <3>(); break;
        case 0x82: set <4>(); break;
        case 0xA2: set <5>(); break;
        case 0xC2: set <6>(); break;
        case 0xE2: set <7>(); break;

        case 0x12: clr <0>(); break;
        case 0x32: clr <1>(); break;
        case 0x52: clr <2>(); break;
        case 0x72: clr <3>(); break;
        case 0x92: clr <4>(); break;
        case 0xB2: clr <5>(); break;
        case 0xD2: clr <6>(); break;
        case 0xF2: clr <7>(); break;

        case 0x0E: tset1(); break;
        case 0x4E: tclr1(); break;

        case 0x5D: mov <SPC_Operands::Register_x, SPC_Operands::Register_a>(); break;
        case 0xBD: mov <SPC_Operands::Register_sp, SPC_Operands::Register_x>(); break;
        case 0xCD: mov <SPC_Operands::Register_x, SPC_Operands::Immediate>(); break;
        case 0xF8: mov <SPC_Operands::Register_x, SPC_Operands::Direct_byte>(); break;

        case 0x7D: mov <SPC_Operands::Register_a, SPC_Operands::Register_x>(); break;
        case 0xDD: mov <SPC_Operands::Register_a, SPC_Operands::Register_y>(); break;
        case 0xE4: mov <SPC_Operands::Register_a, SPC_Operands::Direct_byte>(); break;
        case 0xE8: mov <SPC_Operands::Register_a, SPC_Operands::Immediate>(); break;

        case 0x8D: mov <SPC_Operands::Register_y, SPC_Operands::Immediate>(); break;
        case 0xEB: mov <SPC_Operands::Register_y, SPC_Operands::Direct_byte>(); break;
        case 0xFD: mov <SPC_Operands::Register_y, SPC_Operands::Register_a>(); break;

        case 0xC4: mov_mem <SPC_AddressingModes::Direct, SPC_Operands::Register_a>(); break;
        case 0xC5: mov_mem <SPC_AddressingModes::Absolute, SPC_Operands::Register_a>(); break;
        case 0xC6: mov_mem <SPC_AddressingModes::Indirect, SPC_Operands::Register_a>(); break;
        case 0xC7: mov_mem <SPC_AddressingModes::Direct_x, SPC_Operands::Register_a>(); break;
        case 0xC9: mov_mem <SPC_AddressingModes::Absolute, SPC_Operands::Register_x>(); break;
        case 0xCB: mov_mem <SPC_AddressingModes::Direct, SPC_Operands::Register_y>(); break;
        case 0xCC: mov_mem <SPC_AddressingModes::Absolute, SPC_Operands::Register_y>(); break;
        case 0xD4: mov_mem <SPC_AddressingModes::Direct_x, SPC_Operands::Register_a>(); break;
        case 0xD5: mov_mem <SPC_AddressingModes::Absolute_x, SPC_Operands::Register_a>(); break;
        case 0xD6: mov_mem <SPC_AddressingModes::Absolute_y, SPC_Operands::Register_a>(); break;
        case 0xD7: mov_mem <SPC_AddressingModes::Indirect_y, SPC_Operands::Register_a>(); break;
        case 0xD8: mov_mem <SPC_AddressingModes::Direct, SPC_Operands::Register_x>(); break;
        case 0xDB: mov_mem <SPC_AddressingModes::Direct_x, SPC_Operands::Register_y>(); break;
        case 0x8F: mov_mem <SPC_AddressingModes::Direct, SPC_Operands::Immediate>(); break;

        case 0xE5: mova_mem <SPC_AddressingModes::Absolute>(); break;
        case 0xE7: mova_mem <SPC_AddressingModes::Direct_indirect_x>(); break;
        case 0xF4: mova_mem <SPC_AddressingModes::Direct_x>(); break;
        case 0xF5: mova_mem <SPC_AddressingModes::Absolute_x>(); break;
        case 0xF6: mova_mem <SPC_AddressingModes::Absolute_y>(); break;
        case 0xF7: mova_mem <SPC_AddressingModes::Indirect_y>(); break;
        case 0xE9: movx_mem <SPC_AddressingModes::Absolute>(); break;
        case 0xEC: movy_mem <SPC_AddressingModes::Absolute>(); break;
        case 0xFB: movy_mem <SPC_AddressingModes::Direct_x>(); break;

        case 0xE6: a = read (x + dpOffset); setNZ(a); break; // mov a, (x)
        case 0xBF: a = read (x + dpOffset); x++; setNZ(a); break; // mov a, (x++)

        case 0xFA: { // mov (dd), (ds)
            const auto source = getOperand <SPC_Operands::Direct_byte>();
            const auto destAddress = getAddress <SPC_AddressingModes::Direct>(); 
            write (destAddress, source);
        } break;

        case 0xAF: write (x + dpOffset, a); x++; break; // mov (x+), a

        case 0xBA: mov_ya_dp(); break;
        case 0xDA: mov_dp_ya(); break;

        case 0x1D: x = dec(x); break; // dec x
        case 0x9C: a = dec(a); break; // dec a
        case 0xDC: y = dec(y); break; // dec y
        
        case 0x3D: x = inc(x); break; // inc x
        case 0xBC: a = inc(a); break; // inc a
        case 0xFC: y = inc(y); break; // inc y

        case 0x3A: incw(); break;
        case 0xAB: inc_mem <SPC_AddressingModes::Direct>(); break;
        case 0xAC: inc_mem <SPC_AddressingModes::Absolute>(); break;
        case 0xBB: inc_mem <SPC_AddressingModes::Direct_x>(); break;

        case 0x1A: decw(); break;
        case 0x8B: dec_mem <SPC_AddressingModes::Direct>(); break;
        case 0x8C: dec_mem <SPC_AddressingModes::Absolute>(); break;
        case 0x9B: dec_mem <SPC_AddressingModes::Direct_x>(); break;

        case 0x69: cmp_mem_reg <SPC_AddressingModes::Direct, SPC_Operands::Direct_byte>(); break;
        case 0x78: cmp_mem_reg <SPC_AddressingModes::Direct, SPC_Operands::Immediate>(); break;
                
        case 0x1E: cmp_reg_mem <SPC_Operands::Register_x, SPC_AddressingModes::Absolute>(); break;
        case 0x3E: cmp_reg_mem <SPC_Operands::Register_x, SPC_AddressingModes::Direct>(); break;
        case 0x5E: cmp_reg_mem <SPC_Operands::Register_y, SPC_AddressingModes::Absolute>(); break;
        case 0x64: cmp_reg_mem <SPC_Operands::Register_a, SPC_AddressingModes::Direct>(); break;
        case 0x65: cmp_reg_mem <SPC_Operands::Register_a, SPC_AddressingModes::Absolute>(); break;
        case 0x75: cmp_reg_mem <SPC_Operands::Register_a, SPC_AddressingModes::Absolute_x>(); break;
        case 0x76: cmp_reg_mem <SPC_Operands::Register_a, SPC_AddressingModes::Absolute_y>(); break;
        case 0x7E: cmp_reg_mem <SPC_Operands::Register_y, SPC_AddressingModes::Direct>(); break;

        case 0x68: cmp_reg <SPC_Operands::Register_a, SPC_Operands::Immediate>(); break;
        case 0xAD: cmp_reg <SPC_Operands::Register_y, SPC_Operands::Immediate>(); break;
        case 0xC8: cmp_reg <SPC_Operands::Register_x, SPC_Operands::Immediate>(); break;

        case 0x0B: asl_mem <SPC_AddressingModes::Direct>(); break;
        case 0x0C: asl_mem <SPC_AddressingModes::Absolute>(); break;
        case 0x1B: asl_mem <SPC_AddressingModes::Direct_x>(); break;
        case 0x1C: asl_accumulator(); break;

        case 0x4B: lsr_mem <SPC_AddressingModes::Direct>(); break;
        case 0x4C: lsr_mem <SPC_AddressingModes::Absolute>(); break;
        case 0x5C: lsr_accumulator(); break;

        case 0x6B: ror_mem <SPC_AddressingModes::Direct>(); break;
        case 0x6C: ror_mem <SPC_AddressingModes::Absolute>(); break;
        case 0x7C: ror_accumulator(); break;

        case 0x04: ora <SPC_AddressingModes::Direct>(); break;
        case 0x05: ora <SPC_AddressingModes::Absolute>(); break;
        case 0x06: ora <SPC_AddressingModes::Indirect>(); break;
        case 0x07: ora <SPC_AddressingModes::Direct_indirect_x>(); break;
        case 0x08: ora_imm(); break;
        case 0x09: or_dp <SPC_Operands::Direct_byte>(); break;
        case 0x14: ora <SPC_AddressingModes::Direct_x>(); break;
        case 0x15: ora <SPC_AddressingModes::Absolute_x>(); break;
        case 0x16: ora <SPC_AddressingModes::Absolute_y>(); break;
        case 0x17: ora <SPC_AddressingModes::Indirect_y>(); break;
        case 0x18: or_dp <SPC_Operands::Immediate>(); break;
        case 0x19: or_ip_ip(); break;

        case 0x24: anda <SPC_AddressingModes::Direct>(); break;
        case 0x25: anda <SPC_AddressingModes::Absolute>(); break;
        case 0x26: anda <SPC_AddressingModes::Indirect>(); break;
        case 0x27: anda <SPC_AddressingModes::Direct_indirect_x>(); break;
        case 0x28: anda_imm(); break;
        case 0x29: and_dp <SPC_Operands::Direct_byte>(); break;
        case 0x34: anda <SPC_AddressingModes::Direct_x>(); break;
        case 0x35: anda <SPC_AddressingModes::Absolute_x>(); break;
        case 0x36: anda <SPC_AddressingModes::Absolute_y>(); break;
        case 0x37: anda <SPC_AddressingModes::Indirect_y>(); break;
        case 0x38: and_dp <SPC_Operands::Immediate>(); break;
        case 0x39: and_ip_ip(); break;

        case 0x44: eora <SPC_AddressingModes::Direct>(); break;
        case 0x45: eora <SPC_AddressingModes::Absolute>(); break;
        case 0x46: eora <SPC_AddressingModes::Indirect>(); break;
        case 0x47: eora <SPC_AddressingModes::Direct_indirect_x>(); break;
        case 0x48: eora_imm(); break;
        case 0x49: eor_dp <SPC_Operands::Direct_byte>(); break;
        case 0x54: eora <SPC_AddressingModes::Direct_x>(); break;
        case 0x55: eora <SPC_AddressingModes::Absolute_x>(); break;
        case 0x56: eora <SPC_AddressingModes::Absolute_y>(); break;
        case 0x57: eora <SPC_AddressingModes::Indirect_y>(); break;
        case 0x58: eor_dp <SPC_Operands::Immediate>(); break;
        case 0x59: eor_ip_ip(); break;

        case 0x84: adc_mem <SPC_AddressingModes::Direct>(); break;
        case 0x85: adc_mem <SPC_AddressingModes::Absolute>(); break;
        case 0x86: adc_mem <SPC_AddressingModes::Indirect>(); break;
        case 0x87: adc_mem <SPC_AddressingModes::Direct_indirect_x>(); break;
        case 0x88: a = adc(a, nextByte()); break; // adc a, #imm
        case 0x94: adc_mem <SPC_AddressingModes::Direct_x>(); break;
        case 0x95: adc_mem <SPC_AddressingModes::Absolute_x>(); break;
        case 0x96: adc_mem <SPC_AddressingModes::Absolute_y>(); break;
        case 0x97: adc_mem <SPC_AddressingModes::Indirect_y>(); break;
        case 0x98: adc_dp <SPC_Operands::Immediate>(); break; // adc dp, #imm

        case 0xA4: sbc_mem <SPC_AddressingModes::Direct>(); break;
        case 0xA5: sbc_mem <SPC_AddressingModes::Absolute>(); break;
        case 0xA6: sbc_mem <SPC_AddressingModes::Indirect>(); break;
        case 0xA7: sbc_mem <SPC_AddressingModes::Direct_indirect_x>(); break;
        case 0xA8: a = sbc (a, nextByte()); break; // SBC a, #imm
        case 0xB4: sbc_mem <SPC_AddressingModes::Direct_x>(); break;
        case 0xB5: sbc_mem <SPC_AddressingModes::Absolute_x>(); break;
        case 0xB6: sbc_mem <SPC_AddressingModes::Absolute_y>(); break;
        case 0xB7: sbc_mem <SPC_AddressingModes::Indirect_y>(); break;

        case 0x5A: cmpw(); break;
        case 0x7A: addw(); break;
        case 0x9A: subw(); break;
        case 0x9E: div(); break;
        case 0xCF: mul(); break;
        case 0x9F: xcn(); break;

        case 0x0D: push8 (psw.raw); break;
        case 0x2D: push8(a); break;
        case 0x4D: push8(x); break;
        case 0x6D: push8(y); break;
        case 0xAE: a = pop8(); break;
        case 0xCE: x = pop8(); break;
        case 0xEE: y = pop8(); break;

        case 0x8E: psw.raw = pop8(); dpOffset = psw.directPage ? 0x100 : 0; break;

        case 0x10: jumpRelative (!psw.sign); break; // bpl
        case 0x30: jumpRelative (psw.sign); break;  // bmi 
        case 0x50: jumpRelative (!psw.overflow); break; // bvs
        case 0x70: jumpRelative (psw.overflow); break; // bcv
        case 0x90: jumpRelative (!psw.carry); break; // bcc
        case 0xB0: jumpRelative (psw.carry); break; // bns
        case 0xD0: jumpRelative (!psw.zero); break; // bne
        case 0xF0: jumpRelative (psw.zero); break; // beq
        case 0x2F: jumpRelative <false> (true); break; // bra
        case 0x1F: pc = read16 (getAddress <SPC_AddressingModes::Absolute_x>()); break; // jmp [abs+x]
        case 0x3F: call (nextWord()); break; // call abs
        case 0x5F: pc = read16 (pc); break; // jmp abs
        case 0x6F: ret(); break;

        case 0x03: bbs<0>(); break;
        case 0x23: bbs<1>(); break;
        case 0x43: bbs<2>(); break;
        case 0x63: bbs<3>(); break;
        case 0x83: bbs<4>(); break;
        case 0xA3: bbs<5>(); break;
        case 0xC3: bbs<6>(); break;
        case 0xE3: bbs<7>(); break;

        case 0x13: bbc<0>(); break;
        case 0x33: bbc<1>(); break;
        case 0x53: bbc<2>(); break;
        case 0x73: bbc<3>(); break;
        case 0x93: bbc<4>(); break;
        case 0xB3: bbc<5>(); break;
        case 0xD3: bbc<6>(); break;
        case 0xF3: bbc<7>(); break;

        case 0x2E: cbne <SPC_AddressingModes::Direct>(); break;
        case 0xDE: cbne <SPC_AddressingModes::Direct_x>(); break;

        case 0x01: call (read16(0xFFDE)); break; // TCALL 0
        case 0x11: call (read16(0xFFDC)); break; // TCALL 1
        case 0x21: call (read16(0xFFDA)); break; // TCALL 2
        case 0x31: call (read16(0xFFD8)); break; // TCALL 3
        case 0x41: call (read16(0xFFD6)); break; // TCALL 4
        case 0x51: call (read16(0xFFD4)); break; // TCALL 5
        case 0x61: call (read16(0xFFD2)); break; // TCALL 6
        case 0x71: call (read16(0xFFD0)); break; // TCALL 7
        case 0x81: call (read16(0xFFCE)); break; // TCALL 8
        case 0x91: call (read16(0xFFCC)); break; // TCALL 9
        case 0xA1: call (read16(0xFFCA)); break; // TCALL 10
        case 0xB1: call (read16(0xFFC8)); break; // TCALL 11
        case 0xC1: call (read16(0xFFC6)); break; // TCALL 12
        case 0xD1: call (read16(0xFFC4)); break; // TCALL 13
        case 0xE1: call (read16(0xFFC2)); break; // TCALL 14
        case 0xF1: call (read16(0xFFC0)); break; // TCALL 15
        case 0xEF: case 0xFF: Helpers::panic ("[SPC700] Hanging SPC instruction {} at PC: {:04X}\n", opcode, pc - 1); break; // Sleep, stop

        case 0x6E: dbnz_dp(); break;
        case 0xFE: dbnz_y(); break;

        case 0x00: break; // nop
        case 0x60: psw.carry = false; break; // clrc
        case 0x80: psw.carry = true; break; // setc
        case 0xE0: psw.halfCarry = false; psw.overflow = false; break; // clrv
        case 0x20: psw.directPage = false; dpOffset = 0; break; // clrp
        case 0x40: psw.directPage = true; dpOffset = 0x100; break; // setp
        case 0xC0: psw.interruptEnable = false; break; // di
        case 0xA0: psw.interruptEnable = true; break; // ei
        case 0xED: psw.carry = !psw.carry; break; // notc

        case 0x0A: { // OR1 c, mem, bit
            const auto imm = nextWord();

            const auto bit = imm >> 13; // Top 3 bits of imm are a bit index
            const auto val = read (imm & 0x1FFF); // Low 13 bits of immediate are a memory address
            psw.carry = psw.carry || (Helpers::isBitSet(val, bit));
        } break;

        case 0x2A: { // OR1 !c, mem, bit
            const auto imm = nextWord();

            const auto bit = imm >> 13; // Top 3 bits of imm are a bit index
            const auto val = read (imm & 0x1FFF); // Low 13 bits of immediate are a memory address
            psw.carry = !(psw.carry || Helpers::isBitSet(val, bit));
        } break;

        case 0x4A: { // AND1 c, mem, bit
            const auto imm = nextWord();

            if (psw.carry) {
                const auto bit = imm >> 13; // Top 3 bits of imm are a bit index
                const auto val = read (imm & 0x1FFF); // Low 13 bits of immediate are a memory address
                psw.carry = Helpers::isBitSet(val, bit);
            }
        } break;

        case 0x6A: { // AND1 !c, mem, bit
            const auto imm = nextWord();

            if (psw.carry) {
                const auto bit = imm >> 13; // Top 3 bits of imm are a bit index
                const auto val = read (imm & 0x1FFF); // Low 13 bits of immediate are a memory address
                psw.carry = !Helpers::isBitSet(val, bit);
            }
        } break;

        case 0x8A: { // EOR1 c, mem, bit
            const auto imm = nextWord();
            const auto bit = imm >> 13; // Top 3 bits of imm are a bit index
            const auto val = read (imm & 0x1FFF); // Low 13 bits of immediate are a memory address

            if (Helpers::isBitSet(val, bit)) 
                psw.carry = !psw.carry;
        } break;

        default: Helpers::panic ("[SPC700] Unimplemented opcode: {:02X}\n", opcode); break;
    }
}

// Run the SPC700 until the specified timestamp
void SPC700::runUntil (u64 timestamp) {
    while (cycles < timestamp)
        executeOpcode();
}