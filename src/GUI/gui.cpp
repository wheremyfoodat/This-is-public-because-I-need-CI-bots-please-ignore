#include <filesystem>
#include "tinyfiledialogs.h"
#include "gui.hpp"
#include "snes.hpp"
#include "utils.hpp"

GUI::GUI() : window(sf::VideoMode(800, 600), "SFML window") {
    window.setFramerateLimit(60); // cap FPS to 60
    ImGui::SFML::Init(window);    // Init Imgui-SFML

    auto& io = ImGui::GetIO();  // Set some ImGui options
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    
    ImGui::SFML::UpdateFontTexture(); // Updates font texture
}

void GUI::update() {
    if (running)
        g_snes.runFrame();

    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed)
            window.close();
    }

    ImGui::SFML::Update(window, deltaClock.restart());
 
    showMenuBar();
    if (showCartWindow) 
        showCartInfo();
    if (showRegisterWindow) 
        showRegisters();

    window.clear();
    ImGui::SFML::Render(window);
    window.display();
}

void GUI::showMenuBar() {
    static const char* fileTypes[] = { "*.sfc", "*.smc", "*.snes", "*.zip", "*.rar", "*.7z" };
    
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::MenuItem("Open ROM")) {
            auto file = tinyfd_openFileDialog(
                "Gib SNES ROM",
                "",
                6,
                fileTypes,
                "SNES ROMs",
                0);

            auto path = (file == nullptr) ? std::filesystem::path("") : std::filesystem::path(file); // Check if file dialog was canceled
            Memory::loadROM (path);
            g_snes.reset();
        }

        if (ImGui::BeginMenu("Emulation")) {
            bool cartInserted = Memory::cart.mapper != Mappers::NoCart;

            if (ImGui::MenuItem("Trace", nullptr) && cartInserted) // Make sure not to run without cart
                g_snes.step();
            if (ImGui::MenuItem ("Run", nullptr, &running)) // Same here
                running = running ? cartInserted : false;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem ("Show registers", nullptr, &showRegisterWindow);
            ImGui::MenuItem ("Show cart info", nullptr, &showCartWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void GUI::showRegisters() {
    if (ImGui::Begin("CPU registers")) {
        if (g_snes.cpu.psw.shortAccumulator) // Check the short accumulator bit in PSW to see if a is 8 or 16-bits
            ImGui::Text ("A:  (%02X)%02X", (u8) g_snes.cpu.a.ah, (u8) g_snes.cpu.a.al);
        else
            ImGui::Text ("A: %04X", g_snes.cpu.a.raw);

        ImGui::Text ("X:  %04X", g_snes.cpu.x);
        ImGui::Text ("Y:  %04X", g_snes.cpu.y);
        ImGui::Text ("SP: %04X", g_snes.cpu.sp);
        ImGui::Text ("PC: %04X", g_snes.cpu.pc);
        ImGui::NewLine();

        ImGui::Text ("pb: %02X", g_snes.cpu.pb);
        ImGui::SameLine();
        ImGui::Text ("db: %02X", g_snes.cpu.db);

        // Fetch the PSW bits to display (We can't pass ImGui direct pointers, cause we use bitfields :( )
        bool shortAccumulator = g_snes.cpu.psw.shortAccumulator;
        bool shortIndex = g_snes.cpu.psw.shortIndex;
        bool irqsEnabled = !g_snes.cpu.psw.irqDisable;
        bool decimal = g_snes.cpu.psw.decimal;
        bool zero = g_snes.cpu.psw.zero;
        bool sign = g_snes.cpu.psw.sign;
        bool carry = g_snes.cpu.psw.carry;
        bool overflow = g_snes.cpu.psw.overflow;
        bool emulationMode = g_snes.cpu.emulationMode;

        ImGui::Checkbox ("8-bit accumulator", &shortAccumulator);
        ImGui::SameLine();
        ImGui::Checkbox ("8-bit indexes    ", &shortIndex);
        ImGui::Checkbox ("IRQs enabled     ", &irqsEnabled);
        ImGui::SameLine();
        ImGui::Checkbox ("Decimal mode     ", &decimal);
        ImGui::NewLine();

        ImGui::Checkbox ("Zero    ", &zero);
        ImGui::SameLine();
        ImGui::Checkbox ("Sign    ", &sign);
        ImGui::Checkbox ("Carry   ", &carry);
        ImGui::SameLine();
        ImGui::Checkbox ("Overflow", &overflow);
        ImGui::Checkbox ("Emulation Mode", &emulationMode);
        ImGui::End();
    }
}

void GUI::showCartInfo() {
    if (ImGui::Begin("Cartridge Info")) {
        bool battery = Memory::cart.hasBattery;
        bool rtc = Memory::cart.hasRTC;

        ImGui::Text ("Reset Vector: %04X  IRQ Vector: %04X", Memory::cart.resetVector, Memory::cart.irqVector);
        ImGui::Text ("COP   Vector: %04X  BRK Vector: %04X", Memory::cart.copVector, Memory::cart.brkVector);
        ImGui::Text ("NMI   Vector: %04X", Memory::cart.nmiVector);

        ImGui::NewLine();
        ImGui::Text ("Mapper: %s", Memory::cart.mapperName());
        ImGui::Text ("ROM Size: %dKB (%.2fMB)", Memory::cart.romSize, (float) Memory::cart.romSize / 1024);
        ImGui::Text ("RAM Size: %dKB", Memory::cart.ramSize);
        ImGui::Text ("Expansion Chip: %s", Memory::cart.expansionChipName());
        ImGui::Text ("SHA-1 hash: %s", Memory::cart.sha1_hash.c_str());
        ImGui::NewLine();
        
        ImGui::Checkbox("Battery", &battery);
        ImGui::SameLine();
        ImGui::Checkbox("RTC", &rtc);
        ImGui::End();
    }
}