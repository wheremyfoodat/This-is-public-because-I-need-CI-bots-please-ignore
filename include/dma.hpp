#pragma once
#include "BitField.hpp"
#include "utils.hpp"

union DMAParameters {
    u8 raw;

    BitField <0, 3, u8> unitSelect; // Transfer unit select - shows how large each DMA packet is and the way it's written to the B-Bus
    BitField <3, 2, u8> step; // A-Bus address step (0=Increment, 2=Decrement, 1/3=Fixed)
    BitField <6, 1, u8> addrMode; // (0=Direct Table, 1=Indirect Table)  (HDMA only)
    BitField <7, 1, u8> direction; // Transfer Direction (0=A:CPU to B:I/O, 1=B:I/O to A:CPU) 
};

enum class DMADirection {
    CPUToIO,
    IOToCPU
};

struct DMAChannel {
    uint8_t controlRegs[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    DMAParameters params() { 
        return DMAParameters { .raw = controlRegs[0] };   
    };

    u16 byteCounter() {
        return (controlRegs[6] << 8) | controlRegs[5];
    }

    u32 currentAddress() {
        return (controlRegs[4] << 16) | (controlRegs[3] << 8) | controlRegs[2];
    }

    u32 IOAddress() {
        return 0x2100 + controlRegs[1];
    }
};

namespace Memory {
    void doGPDMA (int channel);
}