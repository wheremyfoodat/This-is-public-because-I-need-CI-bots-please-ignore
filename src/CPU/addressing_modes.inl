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

// There's 3 types of memory accesses that matter for cycle counting
// Read: A value is read from a memory address
// Write: A value is written to a memory address
// RMW (Read-Modify-Write): A value is read from a memory address, modified, and written back
enum class AccessTypes {
    Read, Write, RMW
};

template <AddressingModes addrMode, typename T, AccessTypes type>
u32 getAddress() {
    constexpr bool is16Bit = sizeof(T) == 2;

    if constexpr (addrMode == AddressingModes::Absolute) {
        cycles += is16Bit ? 5 : 4;
        return nextWord() | dbOffset;
    }

    else if constexpr (addrMode == AddressingModes::Absolute_x) {
        cycles += is16Bit ? 5 : 4;
        const auto abs = (u32) nextWord() | dbOffset;
        const auto address = (abs + (u32) x) & 0xFFFFFF; // Note: The address is masked to 24 bits in this case
        
        if constexpr (type == AccessTypes::Write) // Always add a cycle if this is a write operation for... reasons
            cycles += 1;
        else { // Otherwise, add a cycle if PSW.X=0 or if we cross a page boundary
            if (!psw.shortIndex || ((abs & 0xFF00) != (address & 0xFF00)))
                cycles += 1;
        }

        return address;
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

    else if constexpr (addrMode == AddressingModes::Direct) {
        cycles += is16Bit ? 4 : 3;
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        return (u32) nextByte() + (u32) dpOffset;
    }

    else if constexpr (addrMode == AddressingModes::Direct_indirect_x) {
        cycles += is16Bit ? 7 : 6;
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        const auto address = (u32) nextByte() + (u32) dpOffset + (u32) x;
        return (u32) Memory::read16(address) | dbOffset;
    }

    else
        Helpers::panic ("Unknown addressing mode: {}\n", addrMode);
}
