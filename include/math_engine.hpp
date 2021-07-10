#pragma once
#include "utils.hpp"

struct MathEngine {
    u16 quotient = 0;
    u16 dividend = 0;
    u8 divisor = 0;
    u8 multiplier = 0;
    u8 multiplicand = 0;

    u16 division_remainder_multiplication_product = 0; // The remainder for division, or the product for multiplication

    // Mode 7 multiplication registers
    bool m7_multiplicand_latch = false; // 1st write: Low 8 bits of multiplicand, 2nd write: upper 8 bits of multiplicand
    u16 m7_multiplicand = 0;
    u8 m7_multiplier = 0;
    u32 m7_product = 0;
};