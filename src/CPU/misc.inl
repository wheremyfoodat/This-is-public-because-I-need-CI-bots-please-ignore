#pragma once

void wdm() {
    Helpers::warn ("Encountered WDM at PC {:04X}\n", pc - 1);
    pc += 1;
    cycles = 2;
}

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
void phb() { pushR8 (db); }
void phd() {
    push16 (dpOffset);
    cycles = 4;
}

void plb() {
    setDB (pop8<true>()); // Pop DB and set NZ flags
    cycles = 4;
}

void plp() {
    setPSW (pop8<false>());
    Helpers::warn ("PLP at {:04X} Loaded: {:02X}", pc, psw.raw);
    cycles = 4;
}

void pld() {
    dpOffset = pop16<true>();
    cycles = 5;
}

void pha() {
    if (psw.shortAccumulator) {
        push8 (a.al);
        cycles = 3;
    }

    else {
        push16 (a.raw);
        cycles = 4;
    }
}

void phx() {
    if (psw.shortIndex) {
        push8 (x);
        cycles = 3;
    }

    else {
        push16 (x);
        cycles = 4;
    }
}

void phy() {
    if (psw.shortIndex) {
        push8 (y);
        cycles = 3;
    }

    else {
        push16 (y);
        cycles = 4;
    }
}

void plx() {
    if (psw.shortIndex) {
        x = pop8 <true>();
        cycles = 4;
    }

    else {
        x = pop16<true>();
        cycles = 5;
    }
} 

void ply() {
    if (psw.shortIndex) {
        y = pop8 <true>();
        cycles = 4;
    }

    else {
        y = pop16<true>();
        cycles = 5;
    }
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

void pei() {
    const auto pointer = (u32) nextByte() + (u32) dpOffset;
    push16(Memory::read16(pointer));

    if (dpOffset & 0xFF) // Add a cycle if the low byte of d is non-zero
        cycles = 7;
    else 
        cycles = 6;
}

void per() {
    const auto offset = (s16) nextWord();
    push16 (pc + offset);

    cycles = 6;
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