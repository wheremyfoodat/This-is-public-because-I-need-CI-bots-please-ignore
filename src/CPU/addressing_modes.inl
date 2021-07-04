#pragma once

enum class AddressingModes {
    Direct,
    Direct_x,
    Direct_y,
    Direct_indirect,
    Direct_indirect_long,
    Direct_indirect_long_y,
    Direct_indirect_x,
    Direct_indirect_y,
    Absolute,
    Absolute_x,
    Absolute_y,
    Absolute_long,
    Absolute_long_x,
    Absolute_long_y,
    Stack_relative,
    Stack_relative_indirect_indexed,
    Absolute_indirect,
    Absolute_indirect_long,
    Absolute_indirect_x
};

template <AddressingModes addrMode, typename T>
u32 getAddress() {
    constexpr bool is16Bit = sizeof(T) == 2;

    if constexpr (addrMode == AddressingModes::Absolute) {
        cycles += is16Bit ? 5 : 4;
        return nextWord() | dbOffset;
    }

    else if constexpr (addrMode == AddressingModes::Absolute_long) {
        cycles += is16Bit ? 6 : 5;
        const auto offset = nextWord();
        const auto bank = nextByte();

        return (bank << 16) | offset;
    }

    else if constexpr (addrMode == AddressingModes::Absolute_long_x) {
        cycles += is16Bit ? 6 : 5;
        const auto offset = nextWord();
        const auto bank = nextByte();

        return ((bank << 16) | offset) + x;
    }

    else
        Helpers::panic ("Unknown addressing mode: {}\n", addrMode);
}
