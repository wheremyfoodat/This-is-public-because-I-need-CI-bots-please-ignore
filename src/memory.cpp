#include "utils.hpp"
#include "memory.hpp"

using json = nlohmann::json;

// Load ROM and fetch info from database
static void Memory::loadROM (std::filesystem::path directory) {
    if (directory.empty()) // Don't do anything if the directory is empty
        return; 

    auto [file, hash] = Helpers::loadROMWithHash(directory.string()); // Read ROM and get its SHA-1 hash
    cart.rom = file;
    cart.sha1_hash = hash;
    
    if (!gameDB.contains(hash))
        Helpers::panic ("Failed to find game in game db");

    else {
        auto& dbEntry = Memory::gameDB[hash];
        cart.getROMInfo(dbEntry);
    }

    mapFastmemPages(); // Fix our page tables so they fit with our new ROM
}

static void Memory::mapFastmemPages() {
    pageTableRead.fill (nullptr); // Erase page tables
    pageTableWrite.fill (nullptr);

    if (cart.mapper == Mappers::LoROM) { // Map LoROM
        u32 romOffset = 0;

        // Map system area RAM and ROM to fastmem
        for (auto page = 0; page < 0x800;) {
            pageTableRead[page] = &wram[0]; // Mark system area RAM as R/W
            pageTableWrite[page] = &wram[0];
            page += 16; // Skip 32KB ahead, to get to system area ROM
            
            for (auto i = 0; i < 16; i++) { // Map 16 ROM pages
                if (romOffset >= cart.romSize) 
                    break; // Skip mapping ROM pages once we've gone over the ROM size
                
                pageTableRead[page++] = &cart.rom[romOffset];
                romOffset += pageSize;
            }
        }
    }

    else
        Helpers::panic ("Don't know how to map fastmem pages!!! Unknown mapper: %s\n", cart.mapperName());
}

static u8 Memory::read8 (u32 address) {
    const auto page = address >> 11; // Divide address by 2048 to get the page
    const auto pointer = pageTableRead[page];

    if (pointer != nullptr) { // If this is a fast page, read directly
        const auto offset = address & 0x7FF; // Offset inside the page
        return pointer[offset];
    }

    else
        Helpers::panic ("Read from slow address {:06X}\n", address);
}

static void Memory::write8 (u32 address, u8 value) {
    const auto page = address >> 11; // Divide address by 2048 to get the page
    const auto pointer = pageTableWrite[page];

    if (pointer != nullptr) { // If this is a fast page, write directly
        const auto offset = address & 0x7FF; // Offset inside the page
        pointer[offset] = value;
    }

    else
        Helpers::panic ("Wrote to slow address {:06X} (value: {:02X}\n", address, value);
}

static u16 Memory::read16 (u32 address) {
    const auto lsb = read8 (address);
    const auto msb = read8 (address + 1);

    return (msb << 8) | lsb;
}

static void Memory::write16 (u32 address, u16 value) {
    write8 (address, (u8) value);
    write8 (address + 1, value >> 8);
}