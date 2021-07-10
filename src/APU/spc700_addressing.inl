#pragma once

enum class SPC_AddressingModes {
    Indirect,
    Indirect_y,

    Direct,
    Direct_x,
    Direct_y,
    Direct_indirect_x,

    Absolute,
    Absolute_x,
    Absolute_y,
};

enum class SPC_Operands {
    Register_x,
    Register_y,
    Register_a,
    Register_sp,

    Direct_byte,
    Immediate,
};

template <SPC_AddressingModes addrMode>
u16 getAddress() {
    if constexpr (addrMode == SPC_AddressingModes::Absolute)
        return nextWord();
    else if constexpr (addrMode == SPC_AddressingModes::Absolute_x)
        return nextWord() + x;
    else if constexpr (addrMode == SPC_AddressingModes::Absolute_y)
        return nextWord() + y;

    else if constexpr (addrMode == SPC_AddressingModes::Direct)
        return dpOffset + nextByte();
    else if constexpr (addrMode == SPC_AddressingModes::Indirect)
        return dpOffset + x;
    else if constexpr (addrMode == SPC_AddressingModes::Indirect_y)
        return read16 (dpOffset + nextByte()) + y;
    
    else 
        Helpers::panic ("[SPC700] Unimplemented addressing mode {}\n", addrMode);
}

template <SPC_Operands operand>
u8 getOperand() {
    if constexpr (operand == SPC_Operands::Register_a)
        return a;
    else if constexpr (operand == SPC_Operands::Register_x)
        return x;
    else if constexpr (operand == SPC_Operands::Register_y)
        return y;
    else if constexpr (operand == SPC_Operands::Register_sp)
        return sp;
    else if constexpr (operand == SPC_Operands::Immediate)
        return nextByte();
    else if constexpr (operand == SPC_Operands::Direct_byte)
        return read(nextByte() + dpOffset);
    else Helpers::panic ("[SPC700] Unimplemented operand {}\n", operand);
}