#pragma once

void lda_imm() {
    if (psw.shortAccumulator) {
        a.al = nextByte();
        cycles = 2;
    }

    else {
        a.raw = nextWord();
        cycles = 3;
    }
}

void ld_index_imm (u16& reg) {
    if (psw.shortIndex) {
        reg = nextByte();
        cycles = 2;
    }

    else {
        reg = nextWord();
        cycles = 3;
    }
}

void ldx_imm() { ld_index_imm(x); }
void ldy_imm() { ld_index_imm(y); }

template <AddressingModes addrMode>
void sta() {
    if (psw.shortAccumulator) {
        const auto address = getAddress <addrMode, u8>();
        Memory::write8 (address, a.al);
    }

    else {
        const auto address = getAddress <addrMode, u16>();
        Memory::write16 (address, a.raw);
    }
}

template <AddressingModes addrMode>
void stx() {
    if (psw.shortIndex) {
        const auto address = getAddress <addrMode, u8>();
        Memory::write8 (address, x);
    }

    else {
        const auto address = getAddress <addrMode, u16>();
        Memory::write16 (address, x);
    }
}

template <AddressingModes addrMode>
void sty() {
    if (psw.shortIndex) {
        const auto address = getAddress <addrMode, u8>();
        Memory::write8 (address, y);
    }

    else {
        const auto address = getAddress <addrMode, u16>();
        Memory::write16 (address, y);
    }
}

template <AddressingModes addrMode>
void stz() {
    if (psw.shortAccumulator) {
        const auto address = getAddress <addrMode, u8>();
        Memory::write8 (address, 0);
    }

    else {
        const auto address = getAddress <addrMode, u16>();
        Memory::write16 (address, 0);
    }
}