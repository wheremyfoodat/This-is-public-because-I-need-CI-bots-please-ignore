#include "APU/spc700.hpp"

// Run 1 SPC700 opcode
void SPC700::executeOpcode() {
    const auto opcode = nextByte();
    cycles += cycleTable[opcode];

    switch (opcode) {
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

        case 0xBA: mov_ya_dp(); break;
        case 0xDA: mov_dp_ya(); break;

        case 0x1D: x = dec(x); break; // dec x
        case 0x9C: a = dec(a); break; // dec a
        case 0xDC: y = dec(y); break; // dec y
        
        case 0x3D: x = inc(x); break; // inc x
        case 0xBC: a = inc(a); break; // inc a
        case 0xFC: y = inc(y); break; // inc y

        case 0xAB: inc_mem <SPC_AddressingModes::Direct>(); break;
        case 0xAC: inc_mem <SPC_AddressingModes::Absolute>(); break;
        case 0xBB: inc_mem <SPC_AddressingModes::Direct_x>(); break;

        case 0x78: cmp_mem_reg <SPC_AddressingModes::Direct, SPC_Operands::Immediate>(); break;
                
        case 0x5E: cmp_reg_mem <SPC_Operands::Register_y, SPC_AddressingModes::Absolute>(); break;
        case 0x65: cmp_reg_mem <SPC_Operands::Register_a, SPC_AddressingModes::Absolute>(); break;
        case 0x75: cmp_reg_mem <SPC_Operands::Register_a, SPC_AddressingModes::Absolute_x>(); break;
        case 0x7E: cmp_reg_mem <SPC_Operands::Register_y, SPC_AddressingModes::Direct>(); break;

        case 0x68: cmp_reg <SPC_Operands::Register_a, SPC_Operands::Immediate>(); break;
        case 0xAD: cmp_reg <SPC_Operands::Register_y, SPC_Operands::Immediate>(); break;
        case 0xC8: cmp_reg <SPC_Operands::Register_x, SPC_Operands::Immediate>(); break;

        case 0x28: and_imm(); break;

        case 0x0D: push8 (psw.raw); break;
        case 0xAE: a = pop8(); break;
        case 0xCE: x = pop8(); break;
        case 0xEE: y = pop8(); break;

        case 0x10: jumpRelative (!psw.sign); break; // bpl
        case 0x30: jumpRelative (psw.sign); break;  // bmi 
        case 0x50: jumpRelative (!psw.overflow); break; // bvs
        case 0x70: jumpRelative (psw.overflow); break; // bcv
        case 0x90: jumpRelative (!psw.carry); break; // bcc
        case 0xB0: jumpRelative (psw.carry); break; // bns
        case 0xD0: jumpRelative (!psw.zero); break; // bne
        case 0xF0: jumpRelative (psw.zero); break; // beq
        case 0x2F: jumpRelative <false> (true); break; // bra
        case 0x1F: pc = read16(getAddress <SPC_AddressingModes::Absolute_x>()); break; // jmp [abs+x]

        case 0x13: bbc<0>(); break;
        case 0x33: bbc<1>(); break;
        case 0x53: bbc<2>(); break;
        case 0x73: bbc<3>(); break;
        case 0x93: bbc<4>(); break;
        case 0xB3: bbc<5>(); break;
        case 0xD3: bbc<6>(); break;
        case 0xF3: bbc<7>(); break;

        case 0x60: psw.carry = false; break; // clrc
        case 0x80: psw.carry = true; break; // setc
        case 0xE0: psw.halfCarry = false; psw.overflow = false; break; // clrv
        case 0x20: psw.directPage = false; dpOffset = 0; break; // clrp
        case 0x40: psw.directPage = true; dpOffset = 0x100; break; // setp
        case 0xED: psw.carry = !psw.carry; break; // notc

        case 0xFE: dbnz_y(); break;

        default: Helpers::panic ("[SPC700] Unimplemented opcode: {:02X}\n", opcode);
    }
}

// Run the SPC700 until the specified timestamp
void SPC700::runUntil (u64 timestamp) {
    while (cycles < timestamp)
        executeOpcode();
}