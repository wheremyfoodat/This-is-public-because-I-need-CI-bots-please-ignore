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

u8 inc (u8 val) {
    const u8 result = val + 1;
    setNZ (result);

    return result;
}

template <SPC_AddressingModes addrMode>
void inc_mem() {
    const auto address = getAddress <addrMode>();
    u8 value = read (address);

    value = inc (value);
    write (address, value);
}

template <SPC_AddressingModes addrMode, SPC_Operands operand>
void cmp_mem_reg() {
    const auto operand2 = getOperand <operand>();
    const auto address = getAddress <addrMode>();

    const auto operand1 = read (address);
    const auto result = operand1 - operand2;
    setNZ (result);
    psw.carry = operand2 > operand1;
}

template <SPC_Operands operand, SPC_AddressingModes addrMode>
void cmp_reg_mem() {
    const auto operand1 = getOperand <operand>();
    const auto address = getAddress <addrMode>();

    const auto operand2 = read (address);
    const auto result = operand1 - operand2;
    setNZ (result);
    psw.carry = operand2 > operand1;
}

template <SPC_Operands op1, SPC_Operands op2>
void cmp_reg() {
    const auto operand1 = getOperand <op1>();
    const auto operand2 = getOperand <op2>();

    const auto result = operand1 - operand2;
    setNZ (result);
    psw.carry = operand2 > operand1;
}

void and_imm() {
    a &= nextByte();
    setNZ (a);
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
    const auto displacement = (s8) nextByte();
    y -= 1;

    if (y != 0) {
        pc += displacement;
        cycles += 2;
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