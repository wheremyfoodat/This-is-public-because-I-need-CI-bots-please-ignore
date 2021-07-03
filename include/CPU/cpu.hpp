#pragma once
#include <array>
#include "BitField.hpp"
#include "memory.hpp"
#include "utils.hpp"

union PSW {
    u8 raw;

    BitField <0, 1, u8> carry; // Carry flag
    BitField <1, 1, u8> zero;  // Zero flag
    BitField <2, 1, u8> irqDisable; // Interrupt Disable Flag (0 = enable IRQs, 1 = disable IRQs)
    BitField <3, 1, u8> decimal; // Decimal mode flag

    BitField <4, 1, u8> shortIndex; // (0 = 16-bit index registers, 1 = 8-bit index registers)
    BitField <5, 1, u8> shortAccumulator; // (0 = 16-bit index accumulator, 1 = 8-bit index accumulator)
    BitField <6, 1, u8> overflow; // Overflow flag
    BitField <7, 1, u8> sign; // Sign flag
};

union Accumulator {
    u16 raw;

    BitField <0, 8, u16> al; // Low 8 bits of accumulator
    BitField <8, 8, u16> ah; // High 8 bits of accumulator
};

class CPU {
public:
    u16 sp = 0x1FC; // Stack pointer, initialized to 0x1FC on boot (or FC in emulation mode)
    u16 pc = 0;
    u8 pb = 0; // Program bank register
    u8 db = 0; // Data bank register

    PSW psw { .raw = 0x34 }; // Program status word (Boot with IRQs disabled, 8-bit registers)
    Accumulator a { .raw = 0 }; // The accumulator
    u16 x = 0; // Index registers
    u16 y = 0;

    bool emulationMode = true; // Just a stub. We don't actually emulate this because nothing uses it

    void step();
    void reset();

private:
    u32 pbOffset = 0; // pb << 16 and db << 16 respectively
    u32 dbOffset = 0; // Used so we don't have to shift on every memory access

    // Reads a byte from memory[pb:pc] and increments pc by 1
    u8 nextByte() {
        const auto value = Memory::read8 (pbOffset | pc);
        pc += 1;

        return value;
    }

    // Reads a word from memory[pb:pc] and increments pc by 2
    u16 nextWord() {
        const auto value = Memory::read16 (pbOffset | pc);
        pc += 2;

        return value;
    }

    void setPB (u8 value) {
        pb = value;
        pbOffset = pb << 16;
    }

    void setDB (u8 value) {
        db = value;
        dbOffset = db << 16;
    }

    void executeOpcode (u8 opcode);
};