#pragma once
#include "utils.hpp"

// The frequency parameter is actually equal to SPC_CLOCK / TIMER_CLOCK
// So 1024KHz / 8KHz = 128 for Timer 0 and 1, and 1024KHz / 64KHz = 16 for timer 2
template <u64 frequency>
class SPCTimer {
    u8 value = 0; // Actually 4 bits, set to 0 on read
    constexpr static int shift = (frequency == 128) ? 7 : 4; // Instead of dividing by the frequency, we can right shift by 7/4 depending on the timer value

    u64 lastAccessTimestamp = 0; // The timestamp this timer was last accessed
    u64 internalCounter = 0;
    u64 internalCyclesPassed = 0;

public:
    bool enabled = false;
    u16 divider = 256;

    void update (u64 currentTimestamp) {
        if (!enabled) return; // Do not do anything if timer is disabled

        const u64 cyclesPassed = currentTimestamp - lastAccessTimestamp; // Calculate the counter value algorithmically, depending on the divider and how many cycles passed
        internalCyclesPassed += cyclesPassed;

        internalCounter += (internalCyclesPassed >> shift);
        const auto increment = internalCounter / divider;
        
        internalCounter %= divider;
        internalCyclesPassed &= (frequency - 1);

        value = (value + increment) & 0xF; // Increment value and mask to 4 bits
    }

    void disable (u64 currentTimestamp) {
        enabled = false;

        internalCounter = 0;
        internalCyclesPassed = 0;
    }

    // According to documentation, the timer value should be reset when the timer is disabled
    // However, bsnes source indicates this is wrong, and the timer should be reset when enabled
    // https://github.com/bsnes-emu/bsnes/blob/64d484476dd1ff5e94f640ddef5f7233d0404134/bsnes/sfc/smp/io.cpp#L97
    void enable (u64 currentTimestamp) {
        value = 0; // Turning on a timer resets it!
        lastAccessTimestamp = currentTimestamp;
        enabled = true;
    }

    // Reading a timer returns its value, then resets it.
    u8 read() {
        const auto val = value;
        value = 0;
        return val;
    }
};