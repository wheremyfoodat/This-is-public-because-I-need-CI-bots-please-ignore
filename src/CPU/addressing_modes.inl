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

        if constexpr (type == AccessTypes::RMW && !is16Bit)
            cycles += 2;
        else if constexpr (type == AccessTypes::RMW && is16Bit) // Normally being 16-bit adds 2 cycles, but one has already been added already
            cycles += 3;

        return (u32) nextWord() | dbOffset;
    }

    else if constexpr (addrMode == AddressingModes::Absolute_x) {
        cycles += is16Bit ? 5 : 4;
        const auto abs = (u32) nextWord() | dbOffset;
        const auto address = (abs + (u32) x) & 0xFFFFFF; // Note: The address is masked to 24 bits in this case
        
        if constexpr (type == AccessTypes::Write) // Always add a cycle if this is a write operation for... reasons
            cycles += 1;

        else if constexpr (type == AccessTypes::RMW && !is16Bit)
            cycles += 3; // Absolute x-indexed RMW with 8-bit accumulator: 8 cycles

        else if constexpr (type == AccessTypes::RMW && is16Bit) // Normally being 16-bit adds 2 cycles, but one has already been added already
            cycles += 4; // Absolute x-indexed RMW with 16-bit accumulator: 9 cycles
        
        else { // Otherwise, add a cycle if PSW.X=0 or if we cross a page boundary
            if (!psw.shortIndex || ((abs & 0xFF00) != (address & 0xFF00)))
                cycles += 1;
        }

        return address;
    }

    else if constexpr (addrMode == AddressingModes::Absolute_y) {
        cycles += is16Bit ? 5 : 4;
        const auto abs = (u32) nextWord() | dbOffset;
        const auto address = (abs + (u32) y) & 0xFFFFFF; // Note: The address is masked to 24 bits in this case
        
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

        if constexpr (type == AccessTypes::RMW && !is16Bit)
            cycles += 2;
        else if constexpr (type == AccessTypes::RMW && is16Bit) // Normally being 16-bit adds 2 cycles, but one has already been added already
            cycles += 3;

        return (u16) ((u16) nextByte() + dpOffset);
    }

    else if constexpr (addrMode == AddressingModes::Direct_x) {
        cycles += is16Bit ? 5 : 4;
        
        if constexpr (type == AccessTypes::RMW && !is16Bit)
            cycles += 2;
        else if constexpr (type == AccessTypes::RMW && is16Bit) // Normally being 16-bit adds 2 cycles, but one has already been added already
            cycles += 3;
        
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        return (u16) ((u16) nextByte() + dpOffset + x);
    }

    else if constexpr (addrMode == AddressingModes::Direct_y) {
        cycles += is16Bit ? 5 : 4;
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        return (u16) ((u16) nextByte() + dpOffset + y);
    }

    else if constexpr (addrMode == AddressingModes::Direct_indirect) {
        cycles += is16Bit ? 6 : 5;
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        const u16 pointer = (u16) nextByte() + dpOffset;
        return (u32) Memory::read16(pointer) | dbOffset;
    }

    else if constexpr (addrMode == AddressingModes::Direct_indirect_long) {
        cycles += is16Bit ? 7 : 6;
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        const u16 pointer = (u16) nextByte() + dpOffset;
        return (u32) Memory::read16(pointer) | ((u32) Memory::read8 (pointer + 2) << 16);
    }

    else if constexpr (addrMode == AddressingModes::Direct_indirect_x) {
        cycles += is16Bit ? 7 : 6;
        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        const u16 address = (u16) nextByte() + dpOffset + x;
        return (u32) Memory::read16(address) | dbOffset;
    }

    else if constexpr (addrMode == AddressingModes::Direct_indirect_y) {
        cycles += is16Bit ? 6 : 5;
        const u16 pointer = (u16) nextByte() + dpOffset;
        const u32 address = Memory::read16(pointer) | dbOffset;
        const u32 finalAddress = (address + y) & 0xFFFFFF;

        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        if constexpr (type == AccessTypes::Write)
            cycles++;
            
        else { // Otherwise, add a cycle if PSW.X=0 or if we cross a page boundary
            if (!psw.shortIndex || ((finalAddress & 0xFF00) != (address & 0xFF00)))
                cycles += 1;
        }

        return finalAddress;
    }

    else if constexpr (addrMode == AddressingModes::Direct_indirect_long_y) {
        cycles += is16Bit ? 7 : 6;

        if (dpOffset & 0xFF) // add an extra cycle if the low byte of the direct page offset is non-zero
            cycles++;

        const u16 pointer = (u16) nextByte() + dpOffset;
        const u32 address = (u32) Memory::read16(pointer) | (Memory::read8(pointer + 2) << 16);
        return (address + y) & 0xFFFFFF;
    }

    else if constexpr (addrMode == AddressingModes::Stack_relative) {
        cycles += is16Bit ? 5 : 4;

        return ((u32) nextByte() + (u32) sp) & 0xFFFF;
    }

    else if constexpr (addrMode == AddressingModes::Stack_relative_indirect_indexed) {
        cycles += is16Bit ? 8 : 7;

        const u16 pointer = (u16) nextByte() + sp;
        return (Memory::read16(pointer) + dbOffset + y) & 0xFFFFFF;
    }

    else if constexpr (addrMode == AddressingModes::Absolute_indirect) { // This is jmp-only. Cycles are handled in the jmp implementation
        const auto address = nextWord();
        return Memory::read16 (address);
    }

    else if constexpr (addrMode == AddressingModes::Absolute_indirect_x) { // This is jmp-only. Cycles are handled in the jmp implementation
        const auto address = ((nextWord() + x) & 0xFFFF) | pbOffset;
        return Memory::read16 (address);
    }

    else if constexpr (addrMode == AddressingModes::Absolute_indirect_long) { // This is jmp-only. Cycles are handled in the jmp implementation
        const auto address = nextWord();
        return (u32) Memory::read16 (address) | (Memory::read8(address + 2) << 16);
    }

    else
        Helpers::panic ("Unknown addressing mode: {}\n", addrMode);
}
