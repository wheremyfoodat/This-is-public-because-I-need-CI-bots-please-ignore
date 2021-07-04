#pragma once

template <AddressingModes addrMode>
void bit() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8>();
        const auto val = Memory::read8 (addr);

        setNZ8 (val); // Set NZ depending on 8-bit value
        psw.overflow = (val >> 6) & 1; // Set V depending on bit 6
    }

    else {
        const auto addr = getAddress <addrMode, u16>();
        const auto val = Memory::read16 (addr);

        setNZ16 (val); // Set NZ depending on 16-bit value
        psw.overflow = (val >> 14) & 1; // Set V depending on bit 14
    }
}

void sbc_imm() {
    if (psw.shortAccumulator) {
        cycles = 2;
        const auto operand = ~nextByte(); // Invert the subtrahend and treat operation as addition
        u16 result = (u16) a.al + (u16) psw.carry + (u16) operand;

        psw.carry = result >> 8; // Branchless carry calculation
        result &= 0xFF;
        psw.overflow = ((a.al ^ result) & (operand ^ result)) >> 7; // Fast signed overflow calculation
        setNZ8 (result);

        a.al = result;
        if (psw.decimal) Helpers::panic ("Decimal mode 8-bit SBC");
    }

    else {
        cycles = 3;
        const auto operand = ~nextWord();
        u32 result = (u32) a.raw + (u32) psw.carry + (u32) operand;

        psw.carry = result >> 16; // Branchless carry calculation
        result &= 0xFFFF;
        psw.overflow = ((a.raw ^ result) & (operand ^ result)) >> 15; // Fast signed overflow calculation
        setNZ16 (result);

        a.raw = result;

        if (psw.decimal) Helpers::panic ("Decimal mode 16-bit SBC");
    }
}

template <AddressingModes addrMode>
void cmp() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8>();
        const auto val = Memory::read8 (addr);
        
        setNZ8 (a.al - val);
        psw.carry = a.al >= val;
    }

    else {
        const auto addr = getAddress <addrMode, u16>();
        const auto val = Memory::read16 (addr);

        setNZ16 (a.raw - val);
        psw.carry = a.raw >= val;
    }
}