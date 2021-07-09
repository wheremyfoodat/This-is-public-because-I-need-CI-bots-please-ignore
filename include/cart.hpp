#pragma once
#include <filesystem>
#include <vector>
#include "nlohmann/json.hpp"
#include "save_file.hpp"
#include "utils.hpp"

using json = nlohmann::json;

enum class Mappers {
    NoCart,    // No cart inserted
    LoROM,     // LoROM cart
    HiROM,     // HiROM cart
    ExHiROM    // Extended HiROM cart
};

enum class ExpansionChips {
    None, 
    SA_1,
    DSP_1,
    DSP_2,
    DSP_3,
    DSP_4,
    SPC7110,
    SuperFX,
    SuperFX2,
    C4,
    ST018, // Also known as DSP-Seta, programmable ARM CPU
    S_DD1
};

struct Cartridge {
    ExpansionChips secondaryChip = ExpansionChips::None; // Cart coprocessor
    Mappers mapper = Mappers::NoCart; // Cart mapper type

    u32 romSize = 0; // ROM size in kilobytes
    u32 ramSize = 0; // RAM size in kilobytes

    // Exception vectors
    u16 resetVector = 0;
    u16 nmiVector = 0;
    u16 brkVector = 0;
    u16 copVector = 0;
    u16 irqVector = 0;

    std::vector <u8> rom; // The actual contents of the ROM
    std::string sha1_hash = ""; // SHA-1 hash of the ROM used for indexing in the game db
    bool hasBattery = false;
    bool hasRTC = false;

    SaveFile saveFile;
    uint8_t* sram;

    const char* mapperName() {
        switch (mapper) {
            case Mappers::NoCart: return "No cartridge inserted";
            case Mappers::LoROM: return "LoROM";
            case Mappers::HiROM: return "HiROM";
            case Mappers::ExHiROM: return "Extended HiROM";
            
            default: return "Unknown";
        }
    }

    const char* expansionChipName() {
        switch (secondaryChip) {
            case ExpansionChips::None: return "None";
            case ExpansionChips::SA_1: return "SA-1";
            case ExpansionChips::DSP_1: return "DSP-1";
            case ExpansionChips::DSP_2: return "DSP-2";
            case ExpansionChips::DSP_3: return "DSP-3";
            case ExpansionChips::DSP_4: return "DSP-4";
            case ExpansionChips::S_DD1: return "S-DD1";
            case ExpansionChips::C4: return "Capcom CX4";
            case ExpansionChips::SuperFX: return "Super FX (GSU)";
            case ExpansionChips::SuperFX2: return "Super FX 2 (GSU2)";
            case ExpansionChips::ST018: return "ST018 (ARMv3)";

            default: return "Unknown";
        }
    }

    void getROMInfo (json& dbEntry, std::filesystem::path& directory); // Set cartridge info based on game database, initialize SRAM and save files
    void setDefault(); // Set cartridge info to default values if we can't find it in the db
};