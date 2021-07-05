#pragma once

// Exchange carry and emulation mode flags
void xce() {
    const auto tmp = emulationMode;
    emulationMode = psw.carry;
    psw.carry = tmp;

    if (emulationMode)
        Helpers::panic ("Enabled emulation mode!\n");

    cycles = 2;
}

void sec() { psw.carry = true; cycles = 2; }
void clc() { psw.carry = false; cycles = 2; }
void sei() { psw.irqDisable = true; cycles = 2; }
void cli() { psw.irqDisable = false; cycles = 2; }
void cld() { psw.decimal = false; cycles = 2; }
void sed() { psw.decimal = true; cycles = 2; }
void clv() { psw.overflow = false; cycles = 2;}

void pushR8 (u8 value) {
    push8 (value);
    cycles = 3;
}

void php() { pushR8 (psw.raw); }
void phk() { pushR8 (pb); }

void plb() {
    setDB (pop8<true>()); // Pop DB and set NZ flags
    cycles = 4;
}

void pla() {
    if (psw.shortAccumulator) {
        a.al = pop8<true>();
        cycles = 4;
    }

    else {
        a.raw = pop16<true>();
        cycles = 5;
    }
}

void pea() {
    push16(nextWord());
    cycles = 5;
}

void rep() {
    setPSW (psw.raw & ~nextByte());
    cycles = 3;
}

void sep() {
    setPSW (psw.raw | nextByte());
    cycles = 3;
}

template <bool affectFlags>
void transfer (u16& dest, u16 source) {
    dest = source;
    cycles = 2;

    if constexpr (affectFlags)
        setNZ16 (source);
}

void txs() { transfer <false> (sp, x); }
void tcd() { transfer <true> (dpOffset, a.raw); }
void tcs() { transfer <false> (sp, a.raw); }

void tya() {
    if (psw.shortAccumulator) {
        a.al = y & 0xFF;
        setNZ8 (a.al);
    }

    else {
        a.raw = y;
        setNZ16 (a.raw);
    }

    cycles = 2;
}

void tay() {
    if (psw.shortIndex) {
        y = a.al;
        setNZ8 (y);
    }

    else {
        y = a.raw;
        setNZ16 (y);
    }

    cycles = 2;
}

void txa() {
    if (psw.shortAccumulator) {
        a.al = x & 0xFF;
        setNZ8 (a.al);
    }
    
    else {
        a.raw = x;
        setNZ16 (a.raw);
    }

    cycles = 2;
}