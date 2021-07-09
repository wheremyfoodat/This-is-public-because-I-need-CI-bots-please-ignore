#include <cassert>
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"
#include "sha1/sha1.hpp"
#include "utils.hpp"
#include "cart.hpp"
#include "memory.hpp"

// Use a game database to find a ROMs type and attributes through its SHA1 hash
// Also initialize our save file
void Cartridge::getROMInfo (json& dbEntry, std::filesystem::path& directory) {    
    auto expansion = dbEntry["ROMType"].dump();

    // Get coprocessor type
    if (expansion.find("Normal") != std::string::npos) 
        secondaryChip = ExpansionChips::None;
    else if (expansion.find("C4") != std::string::npos)
        secondaryChip = ExpansionChips::C4;
    else if (expansion.find("Super FX2") != std::string::npos)
        secondaryChip = ExpansionChips::SuperFX2;
    else
        Helpers::panic ("Unrecognized coprocessor type.\n{}\n", expansion);

    auto mapperType = dbEntry["Mapper"].dump();
    // Get mapper type
    if (mapperType.find("LoROM") != std::string::npos)
        mapper = Mappers::LoROM;
    else if (mapperType.find("HiROM") != std::string::npos)
        mapper = Mappers::HiROM;
    else if (mapperType.find("Extended HiROM") != std::string::npos)
        mapper = Mappers::ExHiROM;
    else 
        Helpers::panic ("Unrecognized mapper type.\n{}\n", mapperName());

    // Set up exception vectors
    switch (mapper) {
        case Mappers::LoROM:
            resetVector = (rom[0x7FFD] << 8) | rom[0x7FFC];
            copVector = (rom[0x7FE5] << 8) | rom[0x7FE4];
            brkVector = (rom[0x7FE7] << 8) | rom[0x7FE6];
            nmiVector = (rom[0x7FEB] << 8) | rom[0x7FEA];
            irqVector = (rom[0x7FEF] << 8) | rom[0x7FEE];
            break;
        
        case Mappers::HiROM:
        case Mappers::ExHiROM:
            resetVector = (rom[0xFFFD] << 8) | rom[0xFFFC];
            copVector = (rom[0xFFE5] << 8) | rom[0xFFE4];
            brkVector = (rom[0xFFE7] << 8) | rom[0xFFE6];
            nmiVector = (rom[0xFFEB] << 8) | rom[0xFFEA];
            irqVector = (rom[0xFFEF] << 8) | rom[0xFFEE];
        break;
        default: Helpers::panic ("Unknown mapper: {}\n", mapperName());
    }

    romSize = std::stoi (dbEntry["ROMSize"].dump()) * 128; // Convert ROM size to kilobytes from megabits
    ramSize = std::stoi (dbEntry["RAMSize"].dump()) / 8; // Convert RAM size to kilobytes from kilobits
    hasRTC = expansion.find ("RTC") != std::string::npos;
    hasBattery = expansion.find ("Battery") != std::string::npos;

    if (hasBattery) { // Create our memory-mapped save file
        saveFile = SaveFile(directory.stem().replace_extension(".sav"), ramSize * 1024);
        sram = saveFile.data();
    }
}

// Set cart info to default if it wasn't found in the game db
void Cartridge::setDefault() {
    mapper = Mappers::LoROM;
    secondaryChip = ExpansionChips::None;

    romSize = rom.size() / 1024;
    ramSize = 0;
    hasRTC = false;
    hasBattery = false;

    resetVector = (rom[0x7FFD] << 8) | rom[0x7FFC];
    copVector = (rom[0x7FE5] << 8) | rom[0x7FE4];
    brkVector = (rom[0x7FE7] << 8) | rom[0x7FE6];
    nmiVector = (rom[0x7FEB] << 8) | rom[0x7FEA];
    irqVector = (rom[0x7FEF] << 8) | rom[0x7FEE];
}