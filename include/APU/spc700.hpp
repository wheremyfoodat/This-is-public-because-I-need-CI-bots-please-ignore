#pragma once
#include <array>
#include "BitField.hpp"
#include "utils.hpp"
#include "APU/timers.hpp"

union SPC_PSW {
    u8 raw;

    BitField <7, 1, u8> sign; // Set depending on sign bit of last op
    BitField <6, 1, u8> overflow; // Set if last instruction caused signed overflow
    BitField <5, 1, u8> directPage;
    BitField <4, 1, u8> breakFlag; // Set on brk
    BitField <3, 1, u8> halfCarry; // Set if half carry occured
    BitField <2, 1, u8> interruptEnable; // The SNES APU has no interrupt sources, so kinda useless
    BitField <1, 1, u8> zero; // Set if result of last operation is 0
    BitField <0, 1, u8> carry; // Set if carry occured
};

class SPC700 {
    u8 a = 0; // Accumulator
    u8 x = 0, y = 0; // Index registers

    u8 sp = 0; // Stack pointer
    u16 dpOffset = 0; // 0 when PSW.P = 0, 0x100 when PSW.P = 1
    u16 pc = 0xFFC0; // APU reset vector
    SPC_PSW psw { .raw = 0 }; // Program Status Word
    u64 cycles = 0; // Current SPC700 timestamp
    
    const u8 bootrom [64] = {205, 239, 189, 232, 0, 198, 29, 208, 252, 143, 170, 244, 143, 187, 245, 120, 204, 244, 208, 251, 47, 25, 235, 244, 208, 252, 126, 244, 208, 11, 228, 245, 203, 244, 215, 0, 252, 208, 243, 171, 1, 16, 239, 126, 244, 16, 235, 186, 246, 218, 0, 186, 244, 196, 244, 221, 93, 208, 219, 31, 0, 0, 192, 255 };
    std::array <u8, 64 * 1024> ram;

     // SPC timers. The template arguments are the frequencies, calculated as SPC_CLOCK / TIMER_CLOCK
    SPCTimer <1024000 / 8000> timer0; 
    SPCTimer <1024000 / 8000> timer1; 
    SPCTimer <1024000 / 64000> timer2;

    u8 dspRegisterIndex = 0; // Which DSP register to read/write?
    bool bootromMapped = true; // Is FFC0 - FFFF mapped to the bootrom or RAM for reads?

    u8 read (u16 address);
    u16 read16 (u16 address);
    void write (u16 address, u8 value);
    
    u8 nextByte() {
        const u8 val = read (pc);
        pc += 1;
        return val;
    }

    u16 nextWord() {
        const u16 val = read(pc) | (read(pc + 1) << 8);
        pc += 2;
        return val;
    }

    void setNZ (u8 val) {
        psw.sign = val >> 7;
        psw.zero = (val == 0);
    }

    void push8 (u8 val) {
        write (0x100 + sp, val); // The stack is always from 0x100 to 0x1FF!
        sp -= 1;
    }

    u8 pop8() {
        sp += 1;
        return read (0x100 + sp); // The stack is always from 0x100 to 0x1FF!
    }

    const u16 cycleTable[256] = { // Add 2 for taken conditional branches!
        2,8,4,5,3,4,3,6, 2,6,5,4,5,4,6,8,   // $00-$0f
        2,8,4,5,4,5,5,6, 5,5,6,5,2,2,4,6,   // $10-$1f
        2,8,4,5,3,4,3,6, 2,6,5,4,5,4,5,4,   // $20-$2f
        2,8,4,5,4,5,5,6, 5,5,6,5,2,2,3,8,   // $30-$3f
        2,8,4,5,3,4,3,6, 2,6,4,4,5,4,6,6,   // $40-$4f
        2,8,4,5,4,5,5,6, 5,5,4,5,2,2,4,3,   // $50-$5f
        2,8,4,5,3,4,3,6, 2,6,4,4,5,4,5,5,   // $60-$6f
        2,8,4,5,4,5,5,6, 5,5,5,5,2,2,3,6,   // $70-$7f
        2,8,4,5,3,4,3,6, 2,6,5,4,5,2,4,5,   // $80-$8f
        2,8,4,5,4,5,5,6, 5,5,5,5,2,2,12,5,  // $90-$9f
        3,8,4,5,3,4,3,6, 2,6,4,4,5,2,4,4,   // $a0-$af
        2,8,4,5,4,5,5,6, 5,5,5,5,2,2,3,4,   // $b0-$bf
        3,8,4,5,4,5,4,7, 2,5,6,4,5,2,4,9,   // $c0-$cf
        2,8,4,5,5,6,6,7, 4,5,5,5,2,2,6,3,   // $d0-$df
        2,8,4,5,3,4,3,6, 2,4,5,3,4,3,4,3,   // $e0-$ef 
        2,8,4,5,4,5,5,6, 3,4,5,4,2,2,5,3,   // $f0-$ff   
    };

    #include "../../src/APU/spc700_addressing.inl" // Inline files for CPU implementation
    #include "../../src/APU/spc700_instructions.inl"

public:
    u8 inputPorts[4] = { 0, 0, 0, 0 }; // The CPU writes to these ports, the APU reads from them
    u8 outputPorts[4] = { 0, 0, 0, 0 }; // The CPU reads from these ports, the APU writes to them

    void executeOpcode();
    void runUntil (u64 timestamp);
};