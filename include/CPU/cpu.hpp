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
    u16 dpOffset = 0; // Direct page offset

    PSW psw { .raw = 0x34 }; // Program status word (Boot with IRQs disabled, 8-bit registers)
    Accumulator a { .raw = 0 }; // The accumulator
    u16 x = 0; // Index registers
    u16 y = 0;

    bool emulationMode = true; // Just a stub. We don't actually emulate this because nothing uses it
    u32 cycles = 0; // Cycles last instruction took

    void step();
    void reset();

    void fireNMI() {
        irq (Memory::cart.nmiVector);
    }

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

    template <bool setFlags>
    u8 pop8() {
        sp += 1;
        const auto val = Memory::read8 (sp);

        if constexpr (setFlags)
            setNZ8 (val);
        return val;
    }

    template <bool setFlags>
    u16 pop16() {
        u16 val = pop8<false>();
        val |= pop8<false>() << 8;

        if constexpr (setFlags)
            setNZ16 (val);
        return val;
    }

    void push8 (u8 value) {
        Memory::write8 (sp, value);
        sp -= 1;
    }

    void push16 (u16 value) {
        push8 (value >> 8);
        push8 (value & 0xFF);
    }

    // Set the N and Z flags depending on an 8-bit value
    void setNZ8 (u8 value) {
        psw.zero = (value == 0);
        psw.sign = (value >> 7);
    }
    
    // Set the N and Z flags depending on a 16-bit value
    void setNZ16 (u16 value) {
        psw.zero = (value == 0);
        psw.sign = (value >> 15);
    }

    void setPSW (u8 value) {
        psw.raw = value;

        // If the short index bit is turned on, x and y's top bits are cleared. This is NOT true for the accumulator
        if (psw.shortIndex) {
            x &= 0xFF;
            y &= 0xFF;
        }
    }

    void executeOpcode (u8 opcode);

    // Instruction definitions are in inline header file because so templates don't anger the linker
    #include "../../src/CPU/addressing_modes.inl"
    #include "../../src/CPU/loads_stores.inl"
    #include "../../src/CPU/interrupts.inl"
    #include "../../src/CPU/branches.inl"
    #include "../../src/CPU/misc.inl"
    #include "../../src/CPU/alu.inl"
};