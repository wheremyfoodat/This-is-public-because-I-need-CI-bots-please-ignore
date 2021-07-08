#pragma once
#include <SFML/Graphics.hpp>
#include <thread>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_memory_editor.h"

class GUI {
    sf::RenderWindow window;
    sf::Clock deltaClock;
    sf::Texture display;

    MemoryEditor memoryEditor;
    MemoryEditor vramEditor;
    std::thread emuThread;

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
    void showPPURegisters();

    void pingEmuThread();
    void waitEmuThread();

    bool showRegisterWindow = false;
    bool showCartWindow = false;
    bool showMemoryEditor = false;
    bool showVramEditor = false;
    bool showDMAWindow = false;
    bool showPPUWindow = false;

    bool running = false; // Is the emulator running?
    bool vsync = true; // Is vsync enabled?

    int selectedDMAChannel = 0;
};