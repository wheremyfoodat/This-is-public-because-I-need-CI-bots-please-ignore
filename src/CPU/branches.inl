#pragma once

void relativeJump (bool condition) {
    const auto displacement = (s8) nextByte();

    if (condition) {
        pc += displacement;
        cycles = 3;
    }

    else 
        cycles = 2;
}

template <AddressingModes addrMode>
void jmp() {
    const auto addr = getAddress <addrMode, u16, AccessTypes::Read>();
    pc = (u16) addr;

    if constexpr (addrMode == AddressingModes::Absolute_long || addrMode == AddressingModes::Absolute_indirect_long)
        setPB (addr >> 16);
    
    switch (addrMode) { // JMP uses completely different cycle timings per addr mode than literally everything else
        case AddressingModes::Absolute: cycles = 3; break;
        case AddressingModes::Absolute_long: cycles = 4; break;
        case AddressingModes::Absolute_indirect: cycles = 5; break;
        case AddressingModes::Absolute_indirect_x: cycles = 6; break;
        case AddressingModes::Absolute_indirect_long: cycles = 6; break;
    }
}

template <AddressingModes addrMode>
void jsr() {
    const auto addr = getAddress <addrMode, u16, AccessTypes::Read>(); // Get address to jump to
    const auto returnAddr = pc - 1; // pc - 1 is pushed to the stack instead of pc because... reasons?

    if constexpr (addrMode == AddressingModes::Absolute_long) { // Push the pb and fetch new pb if the addr mode is Absolute Long
        push8 (pb);
        setPB (addr >> 16);
    }

    push16 (returnAddr); // Push return address
    pc = (u16) addr;

    switch (addrMode) { // JSR uses completely different cycle timings again
        case AddressingModes::Absolute: cycles = 6; break;
        case AddressingModes::Absolute_long: cycles = 8; break;
        case AddressingModes::Absolute_indirect_x: cycles = 8; break;
    }
}

void brl() {
    const auto offset = (s16) nextWord();
    pc += offset;
}

void rts() {
    pc = pop16<false>() + 1; // JSR pushes the return address - 1, while rts jump to the popped address + 1
    cycles = 6;
}