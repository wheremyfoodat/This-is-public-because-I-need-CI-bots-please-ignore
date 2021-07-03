#pragma once

void ldx_imm() {
    if (psw.shortIndex) {
        x = nextByte();
        cycles = 2;
    }

    else {
        x = nextWord();
        cycles = 3;
    }
}

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