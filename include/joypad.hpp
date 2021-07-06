#pragma once
#include <SFML/Graphics.hpp>
#include "utils.hpp"

namespace Joypads {
    extern u8 hvbjoy;
    extern u16 pad1;

    const sf::Keyboard::Key keyMappings[] = {
        sf::Keyboard::R, // R
        sf::Keyboard::L, // L
        sf::Keyboard::Z, // X
        sf::Keyboard::A, // A
        sf::Keyboard::Right, // Right
        sf::Keyboard::Left, // Left
        sf::Keyboard::Down, // Down
        sf::Keyboard::Up, // Up
        sf::Keyboard::Enter, // Start
        sf::Keyboard::Backspace, // Select
        sf::Keyboard::X, // Y
        sf::Keyboard::S// B
    };

    static void update() {
        pad1 = 0;

        for (auto i = 0; i < 12; i++) {
            if (sf::Keyboard::isKeyPressed(keyMappings[i])) // Check if key is pressed, set respective bit in pad register if yes
                pad1 |= (1 << i);
        }

        pad1 <<= 4; // The buttons are in bits 4-15, not 0-11, so shift the button state left by 4
    }
};