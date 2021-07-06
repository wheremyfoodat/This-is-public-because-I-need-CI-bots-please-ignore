#pragma once
#include "memory.hpp"

void irq (u16 vector) {
    push8(pb); // Push PB, PC and flags
    push16(pc);
    push8(psw.raw);

    psw.decimal = false; // Turn off decimal mode, disable IRQs, set PB to 0 and the PC to the exception vector
    psw.irqDisable = true;
    setPB(0);
    pc = vector;
}

void brk() {
    Helpers::panic ("BRK at PC: {:02X}:{:04X}\n", pb, pc - 1);
    pc += 1; // BRK skips a byte before firing an exception - this byte can be used as a comment by the handler
    irq (Memory::cart.brkVector);

    cycles = 8; // 7 in emulation mode, but we don't have that
}

void cop() {
    Helpers::warn ("COP at PC: {:04X}\n", pc - 1);
    pc += 1; // COP skips a byte before firing an exception - this byte can be used as a comment by the handler
    irq (Memory::cart.copVector);

    cycles = 8; // 7 in emulation mode, but we don't have that
}

void stp() {
    Helpers::warn ("STP at PC: {:04X}\ns", pc - 1);
    pc = Memory::cart.resetVector;
    setPB(0);

    cycles = 3; // Not accurate, but whatever, nothing uses STP
}

void wai() {
    Helpers::panic ("WAI at {:02X}:{:04X}\n", pb, pc);
}

void rti() {
    psw.raw = pop8 <false>(); // Pop flags, then pc, then program bank
    pc = pop16 <false>();
    setPB(pop8 <false>());

    cycles = 7; // 6 in emulation mode, but we don't have that
}