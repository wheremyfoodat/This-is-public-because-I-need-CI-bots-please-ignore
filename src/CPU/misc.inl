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