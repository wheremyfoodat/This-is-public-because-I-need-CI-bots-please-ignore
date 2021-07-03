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

    auto font = io.Fonts -> AddFontFromFileTTF("D:\\Repos\\SNES-JIT\\build\\DejaVuSansMono.ttf", 12.f); // Use DejaVu font (TODO: Don't crash if not found)
    if (!font) Helpers::panic ("Couldn't find font2");
    
    ImGui::SFML::UpdateFontTexture(); // Updates font texture
    //ImGui::PushFont(font); // Push new font
}

void GUI::update() {
    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed)
            window.close();
    }

    ImGui::SFML::Update(window, deltaClock.restart());
 
    showMenuBar();
    showCartInfo();
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
    if (!showRegisterWindow) return;

    if (ImGui::Begin("CPU registers")) {
        if (g_snes.cpu.psw.shortAccumulator) // Check the short accumulator bit in PSW to see if a is 8 or 16-bits
            ImGui::Text ("A:  (%02X)%02X", g_snes.cpu.a.value >> 8, g_snes.cpu.a.value & 0xFF);
        else
            ImGui::Text ("A: %04X", g_snes.cpu.a.value);

        ImGui::Text ("X:  %04X", g_snes.cpu.x.value);
        ImGui::Text ("Y:  %04X", g_snes.cpu.y.value);
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

        ImGui::Checkbox ("8-bit accumulator", &shortAccumulator);
        ImGui::SameLine();
        ImGui::Checkbox ("8-bit indexes", &shortIndex);
        ImGui::Checkbox ("IRQs enabled", &irqsEnabled);
        ImGui::SameLine();
        ImGui::Checkbox ("Decimal mode", &decimal);

        ImGui::Checkbox ("Zero", &zero);
        ImGui::SameLine();
        ImGui::Checkbox ("Sign", &sign);
        ImGui::Checkbox ("Carry", &carry);
        ImGui::SameLine();
        ImGui::Checkbox ("Overflow", &overflow);
        ImGui::End();
    }
}

void GUI::showCartInfo() {
    if (!showCartWindow) return;

    if (ImGui::Begin("Cartridge Info")) {
        ImGui::Text ("Reset Vector: %04X  IRQ Vector: %04X", Memory::resetVector, Memory::irqVector);
        ImGui::Text ("COP   Vector: %04X  BRK Vector: %04X", Memory::copVector, Memory::brkVector);
        ImGui::Text ("NMI   Vector: %04X", Memory::nmiVector);

        ImGui::NewLine();
        ImGui::Text ("Mapper: %s", Memory::getMapperName(Memory::mapper).c_str());
        ImGui::Text ("Size:   %.2fKB", (float) Memory::romSize / 1024);
        ImGui::End();
    }
}