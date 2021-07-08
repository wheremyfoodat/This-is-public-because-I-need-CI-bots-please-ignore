#pragma once
#include "utils.hpp"

struct MathEngine {
    u16 quotient = 0;
    u16 dividend = 0;
    u8 divisor = 0;
    u8 multiplier = 0;
    u8 multiplicand = 0;

    u16 division_remainder_multiplication_product = 0; // The remainder for division, or the product for multiplication
};