#pragma once
#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_memory_editor.h"

class GUI {
    sf::RenderWindow window;
    sf::Clock deltaClock;
    sf::Texture display;

    MemoryEditor memoryEditor;
    MemoryEditor vramEditor;

public:
    GUI();

    void update(); // Update the GUI
    bool isOpen() { return window.isOpen(); } // Shows if the GUI window has been closed or not

private:
    void showMenuBar();
    void showRegisters();
    void showCartInfo();
    void showDisplay();
    void showDMAInfo();

    bool showRegisterWindow = false;
    bool showCartWindow = false;
    bool showMemoryEditor = false;
    bool showVramEditor = false;
    bool showDMAWindow = false;
    bool running = false; // Is the emulator running?

    int selectedDMAChannel = 0;
};