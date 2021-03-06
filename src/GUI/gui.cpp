#include <filesystem>
#include "tinyfiledialogs.h"
#include "gui.hpp"
#include "snes.hpp"
#include "utils.hpp"

GUI::GUI() : window(sf::VideoMode(800, 600), "SFML window") {
    window.setFramerateLimit(60); // cap FPS to 60
    ImGui::SFML::Init(window);    // Init Imgui-SFML
    display.create (256, 224);

    auto& io = ImGui::GetIO();  // Set some ImGui options
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

    io.Fonts -> Clear();
    auto fontPath = std::filesystem::current_path() / "DejaVuSansMono.ttf"; // Use Deja Vu Sans Mono as the font if found
    auto font = io.Fonts -> AddFontFromFileTTF (fontPath.string().c_str(), 18.f);
    if (!font) // Fall back to default font if not found
        io.Fonts -> AddFontDefault();
    ImGui::SFML::UpdateFontTexture(); // Updates font texture

    // Configure memory editor
    memoryEditor.ReadFn = &Memory::read8Debugger;
    memoryEditor.WriteFn = &Memory::write8Debugger;

    emuThread = std::thread([&] { g_snes.runAsync(); } ); // Wake up emulator thread
    emuThread.detach();
}

void GUI::update() {
    // Signal the emu thread to wake up
    if (running)
        pingEmuThread();

    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed)
            window.close();
    }

    ImGui::SFML::Update(window, deltaClock.restart());
 
    showMenuBar();
    showDisplay();

    // Display our debugging windows
    if (showRegisterWindow) 
        showRegisters();
    if (showSPCWindow)
        showSPCRegisters();
    if (showCartWindow) 
        showCartInfo();
    if (showDMAWindow)
        showDMAInfo();
    if (showPPUWindow)
        showPPURegisters();
    
    if (showMemoryEditor)
        memoryEditor.DrawWindow ("CPU Memory Editor", nullptr, 0x1000000);
    if (showVramEditor)
        vramEditor.DrawWindow ("VRAM viewer", g_snes.ppu.vram.data(), 0x10000);
    if (showSPCMemory)
        spcEditor.DrawWindow ("SPC Memory Editor", Memory::apu.getRAM(), 0x10000);

    window.clear (sf::Color(0xDEADBEFF)); // Clear window with magenta
    ImGui::SFML::Render(window);
    window.display();

    if (running) { // Wait for the SNES thread to finish running the frame
        Joypads::update(); // Update pads
        waitEmuThread();
        g_snes.ppu.bufferIndex ^= 1; // Swap buffers
    }
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

            if (file != nullptr) {  // Check if file dialog was canceled
                auto path = std::filesystem::path (file);
                Memory::loadROM (path);
                g_snes.reset();
            }
        }

        if (ImGui::BeginMenu("Emulation")) {
            bool cartInserted = Memory::cart.mapper != Mappers::NoCart;

            if (ImGui::MenuItem ("Trace", nullptr) && cartInserted) // Make sure not to run without cart
                g_snes.step();
            if (ImGui::MenuItem ("Run", nullptr, &running)) // Same here
                running = running ? cartInserted : false;
            if (ImGui::MenuItem ("Pause", nullptr) && running) {
                waitEmuThread(); // Wait till the frame finishes
                running = false; // Stop running
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem ("Show registers", nullptr, &showRegisterWindow);
            ImGui::MenuItem ("Show SPC info", nullptr, &showSPCWindow);
            ImGui::MenuItem ("Show cart info", nullptr, &showCartWindow);
            ImGui::MenuItem ("Show DMA info", nullptr, &showDMAWindow);
            ImGui::MenuItem ("Show PPU registers", nullptr, &showPPUWindow);
            ImGui::MenuItem ("Show VRAM editor", nullptr, &showVramEditor);
            ImGui::MenuItem ("Show CPU memory", nullptr, &showMemoryEditor);
            ImGui::MenuItem ("Show SPC memory", nullptr, &showSPCMemory);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Configuration")) {
            if (ImGui::MenuItem ("Vsync", nullptr, &vsync))
                    window.setFramerateLimit(vsync ? 60 : 0);

            ImGui::End();
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
        ImGui::Text ("Direct Page Offset: %04X", g_snes.cpu.dpOffset);

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

        ImGui::Text ("PSW: %02X", g_snes.cpu.psw.raw);
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
        
        if (ImGui::Button("Trace"))
            g_snes.step();
        ImGui::End();
    }
}

// TODO: More SPC stuff
void GUI::showSPCRegisters() {
    if (ImGui::Begin("SPC registers")) {
        ImGui::Text ("A:  %02X", Memory::apu.a);
        ImGui::Text ("X:  %02X", Memory::apu.x);
        ImGui::Text ("Y:  %02X", Memory::apu.y);
        ImGui::Text ("SP: %02X   ", Memory::apu.sp);
        ImGui::SameLine();
        ImGui::Text ("Direct page offset: %04X", Memory::apu.psw.directPage ? 0x100 : 0);
        ImGui::Text ("PC: %04X ", Memory::apu.pc);
        ImGui::SameLine();
        ImGui::Text ("PSW: %02X", Memory::apu.psw.raw);

        bool breakFlag = Memory::apu.psw.breakFlag;
        bool carry = Memory::apu.psw.carry;
        bool halfCarry = Memory::apu.psw.halfCarry;
        bool sign = Memory::apu.psw.sign;
        bool overflow = Memory::apu.psw.overflow;
        bool zero = Memory::apu.psw.zero;
        bool interruptEnable = Memory::apu.psw.interruptEnable;

        ImGui::Checkbox ("Zero    ", &zero);
        ImGui::SameLine();
        ImGui::Checkbox ("Sign ", &sign);
        ImGui::SameLine();
        ImGui::Checkbox ("Carry", &carry);
        ImGui::SameLine();
        ImGui::Checkbox ("Half Carry", &halfCarry);
        ImGui::Checkbox ("Overflow", &overflow);
        ImGui::SameLine();
        ImGui::Checkbox ("Break", &breakFlag);
        ImGui::SameLine();
        ImGui::Checkbox ("Interrupt enable", &interruptEnable);

        if (ImGui::Button ("Step") && Memory::cart.mapper != Mappers::NoCart) // Make sure not to run without cart
            Memory::apu.executeOpcode();

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

void GUI::showDMAInfo() {
    static const char* steps[] = { "Incrementing", "Fixed", "Decrementing", "Fixed" };
    const auto params = Memory::dmaChannels[selectedDMAChannel].params();
    const auto IOAddress = 0x2100 + Memory::dmaChannels[selectedDMAChannel].IOAddress();

    if (ImGui::Begin("DMA Channels")) {
        ImGui::Text ("Direction:      %s", params.direction ? "CPU to IO" : "IO to CPU");
        ImGui::Text ("B-Bus Address:  %04X", IOAddress);
        ImGui::Text ("HDMA Addr Mode: %s", params.addrMode ? "Direct table" : "Indirect table");
        ImGui::Text ("A-bus step:     %s", steps[params.step]);
        ImGui::Text ("Transfer unit:  %d", (u8) params.unitSelect);
        ImGui::SliderInt("Channel", &selectedDMAChannel, 0, 7);
        ImGui::End();
    }
}

void GUI::showPPURegisters() {
    if (ImGui::Begin("PPU registers")) {
        static const char* hvConfigs[] = { "Disabled", "At H=H", "At V=V + H=0", "At H=H + V=V" };
        const auto hvSetting = (g_snes.ppu.nmitimen >> 4) & 3;
        bool joypadPolling = (g_snes.ppu.nmitimen & 1) != 0;
        bool nmiEnabled = (g_snes.ppu.nmitimen & 0x80) != 0;

        ImGui::Text ("NMITIMEN: %02X", g_snes.ppu.nmitimen);
        ImGui::Text ("H/V IRQ setting: %s", hvConfigs[hvSetting]);
        ImGui::Text ("BG mode: %d", (int) g_snes.ppu.bgmode.mode);
        ImGui::Text ("TM: %02X", g_snes.ppu.tm);
        
        ImGui::Checkbox ("NMIs enabled", &nmiEnabled);
        ImGui::SameLine();
        ImGui::Checkbox ("Automatic pad polling", &joypadPolling);
        ImGui::End();
    }
}

void GUI::showDisplay() {
    if (ImGui::Begin("Display")) {
        const auto size = ImGui::GetContentRegionAvail();
        const auto scale_x = size.x / 256.f;
        const auto scale_y = size.y / 224.f;
        const auto scale = scale_x < scale_y ? scale_x : scale_y;

        display.update(g_snes.ppu.buffers[g_snes.ppu.bufferIndex ^ 1]); // Present the buffer that's not being currently written to
        sf::Sprite sprite (display);
        sprite.setScale (scale, scale);
        
        ImGui::Image(sprite);
        ImGui::End();
    }
}