#pragma once

template <SPC_Operands dest, SPC_Operands source>
void mov() {
    const auto val = getOperand <source>();
    setNZ (val);

    switch (dest) {
        case SPC_Operands::Register_a: a = val; break;
        case SPC_Operands::Register_x: x = val; break;
        case SPC_Operands::Register_y: y = val; break;
        case SPC_Operands::Register_sp: sp = val; break;
        default: Helpers::panic ("[SPC700] Unimplemented mov operand\n");
    }    
}

template <SPC_AddressingModes dest, SPC_Operands source>
void mov_mem() {
    const auto val = getOperand <source>();
    const auto address = getAddress <dest>();

    write (address, val);
}

template <SPC_AddressingModes addrMode>
void mova_mem() {
    const auto address = getAddress <addrMode>();
    const auto val = read (address);
    setNZ (val);
    a = val;
}

template <SPC_AddressingModes addrMode>
void movx_mem() {
    const auto address = getAddress <addrMode>();
    const auto val = read (address);
    setNZ (val);
    x = val;
}

template <SPC_AddressingModes addrMode>
void movy_mem() {
    const auto address = getAddress <addrMode>();
    const auto val = read (address);
    setNZ (val);
    y = val;
}

void mov_ya_dp() {
    const auto address = getAddress <SPC_AddressingModes::Direct>();
    a = read (address);
    y = read (address + 1);

    psw.zero = (a == 0) && (y == 0);
    psw.sign = y >> 7;
}

void mov_dp_ya() {
    const auto address = getAddress <SPC_AddressingModes::Direct>();

    write (address, a);
    write (address + 1, y);
}

u8 dec (u8 val) {
    const u8 result = val - 1;
    setNZ (result);

    return result;
}

template <SPC_AddressingModes addrMode>
void dec_mem() {
    const auto address = getAddress <addrMode>();
    auto value = read (address); // Read memory value
    value = dec (value); // Decrement and set flags
    write (address, value); // Write back value
}

u8 inc (u8 val) {
    const u8 result = val + 1;
    setNZ (result);

    return result;
}

template <SPC_AddressingModes addrMode>
void inc_mem() {
    const auto address = getAddress <addrMode>();
    auto value = read (address); // Read memory value
    value = inc (value); // Increment and set flags
    write (address, value); // Write back value
}

template <SPC_AddressingModes addrMode, SPC_Operands operand>
void cmp_mem_reg() {
    const auto operand2 = getOperand <operand>();
    const auto address = getAddress <addrMode>();

    const auto operand1 = read (address);
    const auto result = operand1 - operand2;
    setNZ (result);
    psw.carry = operand1 >= operand2;
}

template <SPC_Operands operand, SPC_AddressingModes addrMode>
void cmp_reg_mem() {
    const auto operand1 = getOperand <operand>();
    const auto address = getAddress <addrMode>();

    const auto operand2 = read (address);
    const auto result = operand1 - operand2;
    setNZ (result);
    psw.carry = operand1 >= operand2;
}

template <SPC_Operands op1, SPC_Operands op2>
void cmp_reg() {
    const auto operand1 = getOperand <op1>();
    const auto operand2 = getOperand <op2>();

    const auto result = operand1 - operand2;
    setNZ (result);
    psw.carry = operand1 >= operand2;
}

template <SPC_AddressingModes addrMode>
void anda() {
    const auto value = read (getAddress<addrMode>());
    a &= value;
    setNZ (a);
}

void anda_imm() {
    a &= nextByte();
    setNZ (a);
}

void and_ip_ip() {
    const auto destAddress = x + dpOffset;
    auto operand1 = read (destAddress);
    const auto operand2 = read (y + dpOffset);

    operand1 &= operand2;
    setNZ (operand1);
    write (destAddress, operand1);
}

template <SPC_Operands operand>
void and_dp() {
    const auto operand2 = getOperand <operand>(); 
    const auto destAddress = getAddress <SPC_AddressingModes::Direct>();
    
    auto operand1 = read (destAddress);
    operand1 &= operand2;
    setNZ (operand1);
    write (destAddress, operand1);
}

template <SPC_AddressingModes addrMode>
void ora() {
    const auto value = read (getAddress<addrMode>());
    a |= value;
    setNZ (a);
}

void ora_imm() {
    a |= nextByte();
    setNZ (a);
}

void or_ip_ip() {
    const auto destAddress = x + dpOffset;
    auto operand1 = read (destAddress);
    const auto operand2 = read (y + dpOffset);

    operand1 |= operand2;
    setNZ (operand1);
    write (destAddress, operand1);
}

template <SPC_Operands operand>
void or_dp() {
    const auto operand2 = getOperand <operand>(); 
    const auto destAddress = getAddress <SPC_AddressingModes::Direct>();
    
    auto operand1 = read (destAddress);
    operand1 |= operand2;
    setNZ (operand1);
    write (destAddress, operand1);
}

template <SPC_AddressingModes addrMode>
void eora() {
    const auto value = read (getAddress<addrMode>());
    a ^= value;
    setNZ (a);
}

void eora_imm() {
    a ^= nextByte();
    setNZ (a);
}

void eor_ip_ip() {
    const auto destAddress = x + dpOffset;
    auto operand1 = read (destAddress);
    const auto operand2 = read (y + dpOffset);

    operand1 ^= operand2;
    setNZ (operand1);
    write (destAddress, operand1);
}

template <SPC_Operands operand>
void eor_dp() {
    const auto operand2 = getOperand <operand>(); 
    const auto destAddress = getAddress <SPC_AddressingModes::Direct>();
    
    auto operand1 = read (destAddress);
    operand1 ^= operand2;
    setNZ (operand1);
    write (destAddress, operand1);
}

u8 adc (u8 operand1, u8 operand2, u8 carry) {
    u16 result = (u16) operand1 + (u16) operand2 + (u16) carry;

    psw.halfCarry = ((operand1 & 0xF) + (operand2 & 0xF) + carry) >> 4;
    psw.carry = result >> 8;

    result &= 0xFF;
    setNZ (result);
    psw.overflow = ((operand1 ^ result) & (operand2 ^ result)) >> 7;

    return result;
}

u8 adc (u8 operand1, u8 operand2) {
    return adc (operand1, operand2, psw.carry);
}

u8 sbc (u8 operand1, u8 operand2) {
    return adc (operand1, ~operand2); // Implement sbc as adc with an inverted 2nd operand
}

template <SPC_AddressingModes addrMode>
void adc_mem() {
    const auto value = read (getAddress<addrMode>());
    a = adc (a, value);
}

template <SPC_Operands operand>
void adc_dp() {
    const auto operand2 = getOperand <operand>(); 
    const auto destAddress = getAddress <SPC_AddressingModes::Direct>();
    
    auto operand1 = read (destAddress);
    operand1 = adc (operand1, operand2);
    write (destAddress, operand1);
}

template <SPC_AddressingModes addrMode>
void sbc_mem() {
    const auto value = read (getAddress<addrMode>());
    a = sbc (a, value);
}

template <SPC_AddressingModes addrMode>
void asl_mem() {
    const auto address = getAddress <addrMode>();
    auto val = read (address); // Read memory value
    psw.carry = val >> 7; // Store MSB of value into carry

    val <<= 1; // Left shift
    setNZ (val); // Set other flags
    write (address, val); // Write back value
}

void asl_accumulator() {
    psw.carry = a >> 7; // Store MSB of a into carry
    a <<= 1; // Left shift
    setNZ (a); // Set other flags
}

template <SPC_AddressingModes addrMode>
void lsr_mem() {
    const auto address = getAddress <addrMode>();
    auto val = read (address); // Read memory value
    psw.carry = val & 1; // Store lsb of value into carry

    val >>= 1; // Left shift
    setNZ (val); // Set other flags
    write (address, val); // Write back value
}

void lsr_accumulator() {
    psw.carry = a & 1; // Store LSB of a into carry
    a >>= 1; // Left shift
    setNZ (a); // Set other flags
}

template <SPC_AddressingModes addrMode>
void ror_mem() {
    const u8 carry = psw.carry;
    const auto address = getAddress <addrMode>();
    auto val = read (address); // Read memory value

    psw.carry = val & 1; // Store lsb of value into carry
    val = (val >> 1) | (carry << 7); // Rotate with carry
    setNZ (val); // Set other flags
    write (address, val); // Writeback result
}

void ror_accumulator() {
    const u8 carry = psw.carry;
    psw.carry = a & 1; // Store LSB of a into carry
    a = (a >> 1) | (carry << 7); // Rotate with carry
    setNZ (a); // Set other flags
}

template <SPC_AddressingModes addrMode>
void rol_mem() {
    const u8 carry = psw.carry;
    const auto address = getAddress <addrMode>();
    auto val = read (address); // Read memory value

    psw.carry = val >> 7; // Store lsb of value into carry
    val = (val << 1) | carry; // Rotate with carry
    setNZ (val); // Set other flags
    write (address, val); // Writeback result
}

void rol_accumulator() {
    const u8 carry = psw.carry;
    psw.carry = a >> 7; // Store MSB of a into carry
    a = (a << 1) | carry; // Rotate with carry
    setNZ (a); // Set other flags
}

template <bool addCyclesIfBranched = true> // Unconditional branches shouldn't add cycles!
void jumpRelative (bool condition) {
    const auto displacement = (s8) nextByte();
    if (condition) {
        pc += displacement;
        if constexpr (addCyclesIfBranched)
            cycles += 2;
    }
}

void dbnz_y() {
    const auto displacement = (s8) nextByte();  // Branch displacement
    y -= 1; // Decrement y

    if (y != 0) { // Branch if y is not zero
        pc += displacement;
        cycles += 2;
    }
}

void dbnz_dp() {
    const auto address = nextByte() + dpOffset; // Get direct page address
    const auto displacement = (s8) nextByte(); // Branch displacement
    auto val = read (address); // Read dp value

    val -= 1; // Decrement dp value
    write (address, val); // Write back value

    if (val != 0) { // Branch if not 0
        pc += displacement;
        cycles += 2;
    }
}

template <int bit>
void bbs() {
    const auto val = read (nextByte() + dpOffset);
    const auto displacement = (s8) nextByte();

    if ((val & (1 << bit)) != 0) { // Branch if bit is set
        cycles += 2;
        pc += displacement;
    }
}

template <int bit>
void bbc() {
    const auto val = read (nextByte() + dpOffset);
    const auto displacement = (s8) nextByte();

    if ((val & (1 << bit)) == 0) { // Branch if bit is cleared
        cycles += 2;
        pc += displacement;
    }
}

template <SPC_AddressingModes addrMode>
void cbne() {
    const auto val = read (getAddress<addrMode>());
    const auto displacement = (s8) nextByte();

    if (a != val) {
        cycles += 2;
        pc += displacement;
    }
}

void call (u16 address) {
    push16 (pc);
    pc = address;
}

void ret() {
    pc = pop16();
}

void addw() {
    const auto operand2 = read16 (nextByte() + dpOffset); // Read a word from direct page
    const auto operand2_low = (u8) operand2;
    const auto operand2_high = operand2 >> 8;

    u16 result = a + operand2_low; // Add the bottom bytes of YA and the read word
    const u8 carry = result >> 8; // Calculate carry from this addition
    result = (result & 0xFF) | (adc (y, operand2_high, carry) << 8); // Add the top bytes of YA and the read word, plus the carry from first add
    psw.zero = (result == 0);

    a = result & 0xFF; // Set YA
    y = result >> 8;
}

void subw() {
    const auto operand2 = read16 (nextByte() + dpOffset); // Read a word from direct page
    const auto operand2_low = (u8) operand2;
    const auto operand2_high = operand2 >> 8;
    
    psw.carry = 1; // Break down carry to 2 SBCs like hardware does
    u16 result = sbc (a, operand2_low);
    result |= sbc (y, operand2_high) << 8;

    psw.zero = (result == 0);
    a = result & 0xFF; // Set YA
    y = result >> 8;
}

void cmpw() {
    const u16 ya = (y << 8) | a;
    const u16 operand2 = read16 (nextByte() + dpOffset);
    const u16 result = ya - operand2;

    psw.carry = ya >= operand2;
    psw.zero = result == 0;
    psw.sign = result >> 15;
}

void mul() {
    const u16 result = (u16) y * (u16) a; // u16 YA = y * a
    a = result & 0xFF; // a = low 8 bits of result
    y = result >> 8; // y = top 8 bits

    setNZ (y); // NZ are only set depending on Y in mul!
}

// Based on the division algorithm in bsnes 
// https://github.com/bsnes-emu/bsnes/blob/64d484476dd1ff5e94f640ddef5f7233d0404134/bsnes/processor/spc700/instructions.cpp#L348-L377
// This code relies heavily on C++ integer promotion semantics
// TODO: Fix
void div() {
    u16 ya = (y << 8) | a;

    psw.halfCarry = (x & 0xF) < (y & 0xF); // Calculate half carry flag
    psw.overflow = y >= x; // Calculate overflow flag

    if (y < (x << 1)) {
        a = ya / x; // A = Quotient of ya / x
        y = ya % x; // Y = Remainder of ya / x
    }

    else { // ??????????????
        a = 255 - (ya - (x << 9)) / (256 - x);
        y = x + (ya - (x << 9)) % (256 - x);
    }

    setNZ (a); // Set Zero and Sign flags based on quotient
}

void incw() {
    const auto address = nextByte() + dpOffset; // Get direct page address
    auto val = read16 (address); // Fetch value
    val += 1; // Increment
    psw.zero = val == 0; // Set flags based on 16-bit value
    psw.sign = val >> 15;

    write16 (address, val); // Write back
}

void decw() {
    const auto address = nextByte() + dpOffset; // Get direct page address
    auto val = read16 (address); // Fetch value
    val -= 1; // Decrement
    psw.zero = val == 0; // Set flags based on 16-bit value
    psw.sign = val >> 15;

    write16 (address, val); // Write back
}


template <int bit>
void set() {
    const auto address = nextByte() + dpOffset;
    auto val = read (address);

    val |= 1 << bit; // Set the specified bit in the value
    write (address, val); // Write the bit back
}

template <int bit>
void clr() {
    const auto address = nextByte() + dpOffset;
    auto val = read (address);

    val &= ~(1 << bit); // Clear the specified bit in the value
    write (address, val); // Write the bit back
}

void tclr1() {
    const auto address = getAddress <SPC_AddressingModes::Absolute>();
    const auto value = read (address);

    write (address, value & ~a);
    const u8 result = a - value;
    setNZ (result);
}

void tset1() {
    const auto address = getAddress <SPC_AddressingModes::Absolute>();
    const auto value = read (address);

    write (address, a | value);
    const u8 result = a - value;
    setNZ (result);
}

// Exchange nibbles of a
void xcn() {
    a = (a >> 4) | (a << 4);
    setNZ (a);
}

void brk() {
    push16 (pc); // Push PC and PSW, set PC to BRK vector, enable break flag and disable IRQs
    push8 (psw.raw);
    pc = read16 (0xFFDE);

    psw.breakFlag = true;
    psw.interruptEnable = false;
}