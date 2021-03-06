#include "memory.hpp"
#include "dma.hpp"

void Memory::doGPDMA (int channel) {
    const auto params = dmaChannels[channel].params();
    const auto transferType = params.direction ? DMADirection::IOToCPU : DMADirection::CPUToIO;
    auto counter = dmaChannels[channel].byteCounter();
    auto aBusAddress = dmaChannels[channel].currentAddress();
    auto bBusAddress = dmaChannels[channel].IOAddress();

    int step;

    switch (params.step) { // Calculate the A-Bus address step
        case 0: step = 1; break; // Increment if step == 0
        case 2: step = -1; break; // Decrement if step == 2
        default: step = 0; // Fixed address otherwise 
    }

    fmt::print ("DMA from channel {}.\nByte counter: {:04X}.\nA-Bus address: {:4X}\nB-Bus address: {:4X}\nStep: {}\n", 
    channel, counter ? counter : 0x10000, aBusAddress, bBusAddress, step);

    if (transferType == DMADirection::CPUToIO && params.unitSelect == 0) {
        do {
            writeIODMA (bBusAddress, read8(aBusAddress)); // Copy a byte
            aBusAddress += step;
        } while (--counter); // Using a do-while allows us to copy the hardware, which pre-decrements the byte counter, 
                            // and for this reason transfer 0x10000 bytes when you set the counter to 0
    }

    else if (transferType == DMADirection::IOToCPU && params.unitSelect == 0) {
        do {
            write8 (aBusAddress, readSlow(bBusAddress)); // Copy a byte
            aBusAddress += step;
        } while (--counter); // Using a do-while allows us to copy the hardware, which pre-decrements the byte counter, 
                             // and for this reason transfer 0x10000 bytes when you set the counter to 0
    }

    else if (transferType == DMADirection::CPUToIO && params.unitSelect == 1) {
        step <<= 1;
        if (counter & 1) Helpers::panic ("DMA with unit select = 1 where byte counter is odd");

        do {
            writeIODMA (bBusAddress, read8(aBusAddress));
            writeIODMA (bBusAddress + 1, read8(aBusAddress + 1));

            counter -= 2;
            aBusAddress += step;
        } while (counter);
    }

    else if (transferType == DMADirection::CPUToIO && params.unitSelect == 4) {
        step <<= 2;
        if (counter & 3) Helpers::panic ("DMA with unit select = 4 where byte counter is not a multiple of 4");

        do {
            writeIODMA (bBusAddress, read8(aBusAddress));
            writeIODMA (bBusAddress + 1, read8(aBusAddress + 1));
            writeIODMA (bBusAddress + 2, read8(aBusAddress + 2));
            writeIODMA (bBusAddress + 3, read8(aBusAddress + 3));

            counter -= 4;
            aBusAddress += step;
        } while (counter);
    }

    else if (transferType == DMADirection::IOToCPU && params.unitSelect == 5) {
        step <<= 2;
        if (counter & 3) Helpers::panic ("DMA with unit select = 5 where byte counter is not a multiple of 4");

        do {
            write8 (aBusAddress, readSlow(bBusAddress));
            write8 (aBusAddress + 1, readSlow(bBusAddress + 1));
            write8 (aBusAddress + 2, readSlow(bBusAddress));
            write8 (aBusAddress + 3, readSlow(bBusAddress + 1));

            counter -= 4;
            aBusAddress += step;
        } while (counter);
    }
        
    else
        Helpers::panic ("Unknown unit select for GPDMA: {}\nDirection: {}", params.unitSelect, transferType);
}