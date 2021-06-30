#pragma once
#include "BitField.hpp"
#include "xbyak.h"
#include "utils.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__) // MS ABI stuff
    static const auto param1 = Xbyak::util::rcx;
    static const auto param2 = Xbyak::util::rdx;
    static const auto param3 = Xbyak::util::r8;
    static const auto param4 = Xbyak::util::r9;

    static const auto rA = Xbyak::util::bx;
    static const auto rX = Xbyak::util::di;
    static const auto rY = Xbyak::util::si;
#else
    static const auto param1 = Xbyak::util::rdi;
    static const auto param2 = Xbyak::util::rsi;
    static const auto param3 = Xbyak::util::rdx;
    static const auto param4 = Xbyak::util::rcx;

    static const auto rA = Xbyak::util::bx;
    static const auto rX = Xbyak::util::r12w;
    static const auto rY = Xbyak::util::r13w;
#endif


union PSW {
    u8 raw;

    BitField <0, 1, u32> carry; // Carry flag
    BitField <1, 1, u32> zero;  // Zero flag
    BitField <2, 1, u32> irqDisable; // Interrupt Disable Flag (0 = enable IRQs, 1 = disable IRQs)
    BitField <3, 1, u32> decimal; // Decimal mode flag

    BitField <4, 1, u32> shortIndex; // (0 = 16-bit index registers, 1 = 8-bit index registers)
    BitField <5, 1, u32> shortAccumulator; // (0 = 16-bit index accumulator, 1 = 8-bit index accumulator)
    BitField <6, 1, u32> overflow; // Overflow flag
    BitField <7, 1, u32> sign; // Sign flag
};

template <typename T>
struct Register {
    bool writeback = false; // Do we need to write this register back at the end of the block?
    bool isConstant = false; // Does this register have a constant value known at compile time?
    bool restore = false; // Do we need to restore this register's allocated host reg at the end of the block?
    
    T value = static_cast <T> (0);
};

class CPU {
public:
    u16 sp = 0x1FC; // Stack pointer, initialized to 0x1FC on boot (or FC in emulation mode)
    u16 pc;
    u8 pb = 0; // Program bank register
    u8 db = 0; // Data bank register

    PSW psw { .raw = 0x34 }; // Program status word (Boot with IRQs disabled, 8-bit registers)
    Register <u16> a; // The accumulator
    Register <u16> x; // Index registers
    Register <u16> y;

private:
    bool compiling = false;
};