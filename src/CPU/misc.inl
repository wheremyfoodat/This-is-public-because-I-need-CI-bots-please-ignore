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

void sec() {
    psw.carry = true;
    cycles = 2;
}

void clc() {
    psw.carry = false;
    cycles = 2;
}

void sei() {
    psw.irqDisable = true;
    cycles = 2;
}

void phk() {
    push8 (pb);
    cycles = 3;
}

void php() {
    push8 (psw.raw);
    cycles = 3;
}

void plb() {
    setDB (pop8 <true> ()); // Pop DB and set NZ flags
    cycles = 4;
}

void rep() {
    setPSW (psw.raw & ~nextByte());
    cycles = 3;
}

void sep() {
    setPSW (psw.raw | nextByte());
    cycles = 3;
}

void txs() {
    sp = x;
    cycles = 2;
}

void tcd() {
    dpOffset = a.raw;
    cycles = 2;
}

void tcs() {
    sp = a.raw;
    cycles = 2;
}

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

void dex() {
    x -= 1;

    if (psw.shortIndex) {
        x &= 0xFF;
        setNZ8(x);
    }

    else setNZ16(x);
    cycles = 2;
}

void dey() {
    y -= 1;

    if (psw.shortIndex) {
        y &= 0xFF;
        setNZ8(y);
    }

    else setNZ16(y);
    cycles = 2;
}