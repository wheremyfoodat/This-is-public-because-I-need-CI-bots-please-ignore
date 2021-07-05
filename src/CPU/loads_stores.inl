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

template <AddressingModes addrMode>
void lda() {
    if (psw.shortAccumulator) {
        const auto address = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8(address);

        a.al = val;
        setNZ8 (val);
    }

    else {
        const auto address = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16(address);

        a.raw = val;
        setNZ16 (val);
    }
}

template <AddressingModes addrMode>
void ld_index (u16& reg) {
    if (psw.shortIndex) {
        const auto address = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8(address);

        reg = val;
        setNZ8 (val);
    }

    else {
        const auto address = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16(address);

        reg = val;
        setNZ16 (val);
    }
}

template <AddressingModes addrMode>
void ldx() {
    ld_index <addrMode>(x);
}

template <AddressingModes addrMode>
void ldy() {
    ld_index <addrMode>(y);
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
void storeReg (u16 reg, bool isShort) {
    if (isShort) {
        const auto address = getAddress <addrMode, u8, AccessTypes::Write>();
        Memory::write8 (address, reg & 0xFF);
    }

    else {
        const auto address = getAddress <addrMode, u16, AccessTypes::Write>();
        Memory::write16 (address, reg);
    }
}

template <AddressingModes addrMode>
void sta() {
    storeReg <addrMode> (a.raw, psw.shortAccumulator);
}

template <AddressingModes addrMode>
void stx() {
    storeReg <addrMode> (x, psw.shortIndex);
}

template <AddressingModes addrMode>
void sty() {
    storeReg <addrMode> (y, psw.shortIndex);
}

template <AddressingModes addrMode>
void stz() {
    if (psw.shortAccumulator) {
        const auto address = getAddress <addrMode, u8, AccessTypes::Write>();
        Memory::write8 (address, 0);
    }

    else {
        const auto address = getAddress <addrMode, u16, AccessTypes::Write>();
        Memory::write16 (address, 0);
    }
}