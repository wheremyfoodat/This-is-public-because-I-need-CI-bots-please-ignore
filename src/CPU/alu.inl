#pragma once

template <AddressingModes addrMode>
void bit() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);

        psw.zero = (a.al & val) == 0;
        psw.sign = val >> 7;
        psw.overflow = (val >> 6) & 1; // Set V depending on bit 6
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

        psw.zero = (a.raw & val) == 0;
        psw.sign = val >> 15;
        psw.overflow = (val >> 14) & 1; // Set V depending on bit 14
    }
}

void bit_imm() { // Note: BIT #imm has completely different behavior from the other addressing modes
    if (psw.shortAccumulator) {
        psw.zero = (a.al & nextByte()) == 0;
        cycles = 2;
    }
    
    else {
        psw.zero = (a.raw & nextWord()) == 0;
        cycles = 3;
    }
}

template <AddressingModes addrMode>
void trb() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::RMW>();
        const auto val = Memory::read8 (addr);

        psw.zero = (a.al & val) == 0;
        const auto res = (~a.al) & 0xFF & val;
        Memory::write8 (addr, res);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::RMW>();
        const auto val = Memory::read16 (addr);

        psw.zero = (a.raw & val) == 0;
        const auto res = (~a.raw) & val;
        Memory::write16 (addr, res);
    }
}

template <AddressingModes addrMode>
void tsb() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::RMW>();
        const auto val = Memory::read8 (addr);

        psw.zero = (a.al & val) == 0;
        const auto res = a.al | val;
        Memory::write8 (addr, res);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::RMW>();
        const auto val = Memory::read16 (addr);

        psw.zero = (a.raw & val) == 0;
        const auto res = a.raw | val;
        Memory::write16 (addr, res);
    }
}


template <AddressingModes addrMode>
void ora() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);

        a.al = a.al | val;
        setNZ8 (a.al);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

        a.raw = a.raw | val;
        setNZ16 (a.raw); // Set NZ depending on 16-bit value
    }
}

void ora_imm() {
    if (psw.shortAccumulator) {
        const auto val = nextByte();

        a.al = a.al | val;
        setNZ8 (a.al);
        cycles = 2;
    }

    else {
        const auto val = nextWord();

        a.raw = a.raw | val;
        setNZ16 (a.raw); // Set NZ depending on 16-bit value
        cycles = 3;
    }
}

template <AddressingModes addrMode>
void eor() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);

        a.al = a.al ^ val;
        setNZ8 (a.al);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

        a.raw = a.raw ^ val;
        setNZ16 (a.raw); // Set NZ depending on 16-bit value
    }
}

void eor_imm() {
    if (psw.shortAccumulator) {
        const auto val = nextByte();

        a.al = a.al ^ val;
        setNZ8 (a.al);
        cycles = 2;
    }

    else {
        const auto val = nextWord();

        a.raw = a.raw ^ val;
        setNZ16 (a.raw); // Set NZ depending on 16-bit value
        cycles = 3;
    }
}


template <AddressingModes addrMode>
void and_() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);

        a.al = a.al & val;
        setNZ8 (a.al);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

        a.raw = a.raw & val;
        setNZ16 (a.raw); // Set NZ depending on 16-bit value
    }
}

void and_imm() {
    if (psw.shortAccumulator) {
        const auto val = nextByte();

        a.al = a.al & val;
        setNZ8 (a.al);
        cycles = 2;
    }

    else {
        const auto val = nextWord();

        a.raw = a.raw & val;
        setNZ16 (a.raw); // Set NZ depending on 16-bit value
        cycles = 3;
    }
}

template <bool is16Bit>
void adc (u16 operand) {
    if (psw.decimal) Helpers::panic ("Decimal mode ADC");

    if constexpr (is16Bit) {
        u32 result = (u32) a.raw + (u32) psw.carry + (u32) operand;
        psw.carry = result >> 16;

        result &= 0xFFFF;
        psw.overflow = ((a.raw ^ result) & (operand ^ result)) >> 15; // Fast signed overflow calculation
        setNZ16 (result);
        a.raw = result;
    }

    else {
        u16 result = (u16) a.al + (u16) psw.carry + (u16) operand;

        psw.carry = result >> 8; // Branchless carry calculation
        result &= 0xFF;
        psw.overflow = ((a.al ^ result) & (operand ^ result)) >> 7; // Fast signed overflow calculation
        setNZ8 (result);

        a.al = result;        
    }
}

void adc_imm() {
    if (psw.shortAccumulator) {
        cycles = 2;
        adc <false> (nextByte());
    }

    else {
        cycles = 3;
        adc <true> (nextWord());
    }
}

template <AddressingModes addrMode>
void adc_mem() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);
        
        adc <false> (val);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

       adc <true> (val);
    }
}

template <bool is16Bit>
void sbc (u16 operand) {
    if (psw.decimal) Helpers::panic ("Decimal mode ADC");

    if constexpr (is16Bit) {
        operand = ~operand; // Invert the operand and treat the operation as an addition
        u32 result = (u32) a.raw + (u32) psw.carry + (u32) operand;
        psw.carry = result >> 16;

        result &= 0xFFFF;
        psw.overflow = ((a.raw ^ result) & (operand ^ result)) >> 15; // Fast signed overflow calculation
        setNZ16 (result);
        a.raw = result;
    }

    else {
        operand = (~operand) & 0xFF;
        u16 result = (u16) a.al + (u16) psw.carry + (u16) operand;

        psw.carry = result >> 8; // Branchless carry calculation
        result &= 0xFF;
        psw.overflow = ((a.al ^ result) & (operand ^ result)) >> 7; // Fast signed overflow calculation
        setNZ8 (result);

        a.al = result;        
    }
}

void sbc_imm() {
    if (psw.shortAccumulator) {
        cycles = 2;
        sbc <false> (nextByte());
    }

    else {
        cycles = 3;
        sbc <true> (nextWord());
    }
}

template <AddressingModes addrMode>
void sbc_mem() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);
        
        sbc <false> (val);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

       sbc <true> (val);
    }
}

template <AddressingModes addrMode>
void compare (u16 reg, bool isShort) {
    if (isShort) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::Read>();
        const auto val = Memory::read8 (addr);
        
        const u8 a = reg & 0xFF;
        setNZ8 (a - val);
        psw.carry = a >= val;
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
        const auto val = Memory::read16 (addr);

        setNZ16 (reg - val);
        psw.carry = reg >= val;
    }
}

template <AddressingModes addrMode>
void cmp() {
    compare <addrMode> (a.raw, psw.shortAccumulator);
}

template <AddressingModes addrMode>
void cpx() {
    compare <addrMode> (x, psw.shortIndex);
}

template <AddressingModes addrMode>
void cpy() {
    compare <addrMode> (y, psw.shortIndex);
}

void compare_imm (u16 reg, bool isShort) {
    if (isShort) {
        const auto val = nextByte();
        const u8 a = reg & 0xFF;

        setNZ8 (a - val);
        psw.carry = a >= val;
        cycles = 2;
    }

    else {
        const auto val = nextWord();

        setNZ16 (reg - val);
        psw.carry = reg >= val;
        cycles = 3;
    }
}

void cmp_imm() { compare_imm (a.raw, psw.shortAccumulator); }
void cpx_imm() { compare_imm (x, psw.shortIndex); }
void cpy_imm() { compare_imm (y, psw.shortIndex); }

template <AddressingModes addrMode>
void inc() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::RMW>();
        const auto val = Memory::read8 (addr);

        const u8 res = val + 1;
        setNZ8 (res);
        Memory::write8 (addr, res);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::RMW>();
        const auto val = Memory::read16 (addr);

        const u16 res = val + 1;
        setNZ16 (res);
        Memory::write16 (addr, res);
    }
}

template <AddressingModes addrMode>
void dec() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress <addrMode, u8, AccessTypes::RMW>();
        const auto val = Memory::read8 (addr);

        const u8 res = val - 1;
        setNZ8 (res);
        Memory::write8 (addr, res);
    }

    else {
        const auto addr = getAddress <addrMode, u16, AccessTypes::RMW>();
        const auto val = Memory::read16 (addr);

        const u16 res = val - 1;
        setNZ16 (res);
        Memory::write16 (addr, res);
    }
}

void incIndex (u16& value) {
    value += 1;

    if (psw.shortIndex) {
        value &= 0xFF;
        setNZ8 (value);
    }

    else setNZ16 (value);
    cycles = 2;
}

void decIndex (u16& value) {
    value -= 1;

    if (psw.shortIndex) {
        value &= 0xFF;
        setNZ8 (value);
    }

    else setNZ16 (value);
    cycles = 2;
}

void inx() { incIndex(x); }
void iny() { incIndex(y); }
void dex() { decIndex(x); }
void dey() { decIndex(y); }

void ina() {
    if (psw.shortAccumulator) {
        a.al = a.al + 1;
        setNZ8 (a.al);
    }

    else {
        a.raw = a.raw + 1;
        setNZ16 (a.raw);
    }

    cycles = 2;
}

void dea() {
    if (psw.shortAccumulator) {
        a.al = a.al - 1;
        setNZ8 (a.al);
    }

    else {
        a.raw = a.raw - 1;
        setNZ16 (a.raw);
    }

    cycles = 2;
}

void asl_accumulator() {
    if (psw.shortAccumulator) {
        psw.carry = a.al >> 7;
        a.al = a.al << 1;
        setNZ8 (a.al);
    }

    else {
        psw.carry = a.raw >> 15;
        a.raw <<= 1;
        setNZ16 (a.raw);
    }

    cycles = 2;
}

template <AddressingModes addrMode>
void asl() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress<addrMode, u8, AccessTypes::RMW>();
        auto val = Memory::read8 (addr);

        psw.carry = val >> 7;
        val <<= 1;
        setNZ8 (val);
        Memory::write8 (addr, val);
    }

    else {
        const auto addr = getAddress<addrMode, u16, AccessTypes::RMW>();
        auto val = Memory::read16 (addr);

        psw.carry = val >> 15;
        val <<= 1;
        setNZ16 (val);
        Memory::write16 (addr, val);
    }
}

template <AddressingModes addrMode>
void lsr() {
    if (psw.shortAccumulator) {
        const auto addr = getAddress<addrMode, u8, AccessTypes::RMW>();
        auto val = Memory::read8 (addr);

        psw.carry = val & 1;
        val >>= 1;
        setNZ8 (val);
        Memory::write8 (addr, val);
    }

    else {
        const auto addr = getAddress<addrMode, u16, AccessTypes::RMW>();
        auto val = Memory::read16 (addr);

        psw.carry = val & 1;
        val >>= 1;
        setNZ16 (val);
        Memory::write16 (addr, val);
    }
}

void lsr_accumulator() {
    psw.carry = a.raw & 1;

    if (psw.shortAccumulator) {
        a.al = a.al >> 1;
        setNZ8 (a.al);
    }

    else {
        a.raw >>= 1;
        setNZ16 (a.raw);
    }

    cycles = 2;
}

template <AddressingModes addrMode>
void rol() {
    const auto oldCarry = psw.carry;
    if (psw.shortAccumulator) {
        const auto addr = getAddress<addrMode, u8, AccessTypes::RMW>();
        auto val = Memory::read8 (addr);

        psw.carry = val >> 7;
        val = (val << 1) | oldCarry;
        setNZ8 (val);
        Memory::write8 (addr, val);
    }

    else {
        const auto addr = getAddress<addrMode, u16, AccessTypes::RMW>();
        auto val = Memory::read16 (addr);

        psw.carry = val >> 15;
        val = (val << 1) | oldCarry;
        setNZ16 (val);
        Memory::write16 (addr, val);
    }
}

void rol_accumulator() {
    const bool oldCarry = psw.carry;

    if (psw.shortAccumulator) {
        psw.carry = a.al >> 7;
        a.al = (a.al << 1) | oldCarry;
        setNZ8 (a.al);
    }

    else {
        psw.carry = a.raw >> 15;
        a.raw = (a.raw << 1) | oldCarry;
        setNZ16 (a.raw);
    }

    cycles = 2;
}

template <AddressingModes addrMode>
void ror() {
    const auto oldCarry = psw.carry;
    if (psw.shortAccumulator) {
        const auto addr = getAddress<addrMode, u8, AccessTypes::RMW>();
        auto val = Memory::read8 (addr);

        psw.carry = val & 1;
        val = (val >> 1) | (oldCarry << 7);
        setNZ8 (val);
        Memory::write8 (addr, val);
    }

    else {
        const auto addr = getAddress<addrMode, u16, AccessTypes::RMW>();
        auto val = Memory::read16 (addr);

        psw.carry = val & 1;
        val = (val >> 1) | (oldCarry << 15);
        setNZ16 (val);
        Memory::write16 (addr, val);
    }
}

void ror_accumulator() {
    const bool oldCarry = psw.carry;
    psw.carry = a.raw & 1;

    if (psw.shortAccumulator) {
        a.al = (a.al >> 1) | (oldCarry << 7);
        setNZ8 (a.al);
    }

    else {
        a.raw = (a.raw >> 1) | (oldCarry << 15);
        setNZ16 (a.raw);
    }

    cycles = 2;
}