#include "CPU/cpu.hpp"
#include "utils.hpp"

using JITCallback = uint32_t (*)(); // Our JIT returns the cycle counter in eax

// To index blocks, the dynarec uses a 2-level LUT
// The first level is indexed with a key consisting of (pb << 1) | (msb of pc)
// The second level is indexed with a key consisting of
// (short acc flag << 17) | (short index flag << 16) | (decimal flag << 15) | pc & 0x7FFF
void CPU::runBlock() {
    const auto key1 = (pb << 1) | (pc >> 15);
    const auto key2 = ((psw.raw & 0b111000) << 12) | (pc & 0x7FFF);

    const auto page = recompilerBankLUT[key1]; // Read the top-level LUT, panic if executing from a bad address
    if (page == nullptr)
        Helpers::panic ("Execution from invalid address: %02X:%08X\n", pb, pc);
    
    const auto pointer = page[key2];
    if (pointer == 0)
        Helpers::panic ("Gotta compile blocc");

    const auto callback = (JITCallback) pointer;
    (*callback)(); // Jump to block code
}