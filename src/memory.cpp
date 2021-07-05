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
    
    if (!gameDB.contains(hash)) {
        Helpers::warn ("Failed to find game in game db (Hash: {})\n", hash);
        Helpers::warn ("Defaulting to LoROM, 0KB SRAM\n");
        
        cart.setDefault();
    }

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
        u32 size = cart.romSize * 1024;  // size of the ROM in bytes

        if (cart.rom.size() >= 2 * megabyte) Helpers::panic ("LoROM ROM over 2MB");

        // Map system area RAM and ROM to fastmem
        for (auto page = 0; page < 0x800;) {
            for (auto i = 0; i < 4; i++) { // Map 4 2KB pages to system area RAM
                pageTableRead[page] = &wram[i * pageSize]; // Mark system area RAM as R/W
                pageTableWrite[page] = &wram[i * pageSize];
                    
                pageTableRead[page + 0x1000] = &wram[i * pageSize]; // Map the upper system area mirror as well
                pageTableWrite[page + 0x1000] = &wram[i * pageSize];

                page += 1; 
            }
 
            page += 12; // Skip 24KB ahead, to get to system area ROM
            
            for (auto i = 0; i < 16; i++) { // Map 16 ROM pages
                if (romOffset < size) { // Don't map pages if we've gone over the ROM size
                    pageTableRead[page] = &cart.rom[romOffset];
                    pageTableRead[page + 0x1000] = &cart.rom[romOffset]; // Mark the upper ROM mirror as well
                    romOffset += pageSize;
                }

                page += 1;
            }
        }   
    }

    else
        Helpers::panic ("Don't know how to map fastmem pages!!! Unknown mapper: {}\n", cart.mapperName());
}

static u8 Memory::read8 (u32 address) {
    const auto page = address >> 11; // Divide address by 2048 to get the page
    const auto pointer = pageTableRead[page];

    if (pointer != nullptr) { // If this is a fast page, read directly
        const auto offset = address & 0x7FF; // Offset inside the page
        return pointer[offset];
    }

    else
        return readSlow (address);
}

static void Memory::write8 (u32 address, u8 value) {
    const auto page = address >> 11; // Divide address by 2048 to get the page
    const auto pointer = pageTableWrite[page];

    if (pointer != nullptr) { // If this is a fast page, write directly
        const auto offset = address & 0x7FF; // Offset inside the page
        pointer[offset] = value;
    }

    else
        writeSlow (address, value);
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

//  write function for stuff like IO, where fastmem will not work
// IfSlow "isDebugger" is true, this function does not provoke read side-effects when reading IO
template <bool isDebugger>
static u8 Memory::readSlow (u32 address) {
    const auto bank = address >> 16;
    const auto addr = (u16) address;

    if (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF)) { // See if the address is in system area
        switch (addr) {
            case 0x4210: // rdnmi
                ppu->rdnmi ^= 0x80;
                Helpers::warn ("Read from RDNMI (stubbed)\n");
                return ppu->rdnmi;

            default: Helpers::panic ("Read from unimplemented slow address {:06X}", address);
        }
    }

    else 
        Helpers::panic ("Read from unimplemented slow address {:06X}", address);
}

//  write function for stuff like IO, where fastmem will not work
// IfSlow "isDebugger" is true, this function does not provoke write side-effects when writing to IO
template <bool isDebugger>
static void Memory::writeSlow (u32 address, u8 value) {
    const auto bank = address >> 16;
    const auto addr = (u16) address;

    if (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF)) // See if the address is in system area
        writeIO <false> (addr, value);
    else
        Helpers::panic ("Write to unimplemented slow address {:06X}", address);
}

template <bool isDebugger>
static void Memory::writeIO (u16 address, u8 value) {
    switch (address) {
        case 0x2100: Helpers::warn ("Unimplemented write to INIDISP (val: {:02X})\n", value); break;
        case 0x2101: Helpers::warn ("Unimplemented write to OBJSEL (val: {:02X})\n", value); break;
        case 0x2102: ppu->oamaddr.low = value; break;
        case 0x2103: ppu->oamaddr.high = value; break;
        case 0x2105: ppu->bgmode.raw = value; break;
        case 0x2106: Helpers::warn ("Unimplemented write to mosaic register (val: {:02X})\n", value); break;

        case 0x2107: ppu->sc[0].raw = value; break; // BG1SC
        case 0x2108: ppu->sc[1].raw = value; break; // BG2SC
        case 0x2109: ppu->sc[2].raw = value; break; // BG3SC
        case 0x210A: ppu->sc[3].raw = value; break; // BG4SC

        case 0x210B: // BG12NBA 
            ppu->nba[0] = value & 0xF; 
            ppu->nba[1] = value >> 4;
            break;
            
        case 0x210C: // BG34NBA 
            ppu->nba[2] = value & 0xF; 
            ppu->nba[3] = value >> 4;
            break;

        case 0x2115: // VMAIN
            ppu->vmain.raw = value;
            switch (value & 3) {
                case 0: ppu->vramStep = 1; break;
                case 1: ppu->vramStep = 32; break;
                default: ppu->vramStep = 128; break;
            }

            if (value & 0b1100) // Check if VRAM address translation was enabled and panic
                Helpers::panic ("VRAM address translation\n");

            break;

        case 0x2116: ppu->vmaddr.low = value; break; // VMADDL
        case 0x2117: ppu->vmaddr.high = value; break; // VMADDH
        
        case 0x2118: { // VMDATAL
            const auto vmaddr = ppu->vmaddr.raw & 0x7FFF; // The VRAM address we'll access, masked to 15 bits
            ppu->vram[vmaddr] = (ppu->vram[vmaddr] & 0xFF00) | value; // Write to the low byte of the address

            if (!ppu->vmain.incrementOnHigh) // Increment VRAM address if vmain.7 is not set
                ppu->vmaddr.raw += ppu->vramStep;
        } break;

        case 0x2119: { // VMDATAH
            const auto vmaddr = ppu->vmaddr.raw & 0x7FFF; // The VRAM address we'll access, masked to 15 bits
            ppu->vram[vmaddr] = (ppu->vram[vmaddr] & 0xFF) | (value << 8); // Write to the high byte of the address

            if (ppu->vmain.incrementOnHigh) // Increment VRAM address if vmain.7 is set
                ppu->vmaddr.raw += ppu->vramStep;
        } break;

        case 0x2121: // CGADD
            ppu->paletteAddr = value;
            ppu->paletteLatch = false;
            break;

        case 0x2122: { // CGDATA
            if (ppu->paletteLatch) {
                const auto palette = ((value & 0x7F) << 8) | ppu->latchedPalette; // MSB of palette is ignored
                ppu->paletteRAM[ppu->paletteAddr] = palette;
                ppu->paletteAddr++; 
            }

            else
                ppu->latchedPalette = value;

            ppu->paletteLatch = !ppu->paletteLatch;
        } break;

        case 0x2180: // WMDATA
            wram[wramAddress++] = value;
            wramAddress &= 0x1FFFF;
            break;

        case 0x2181: wramAddress = (wramAddress & ~0xFF) | value; break; // WMADDL
        case 0x2182: wramAddress = (wramAddress & ~0xFF00) | (value << 8); break; // WMADDM;
        case 0x2183: wramAddress = (wramAddress & 0xFFFF) | ((value & 1) << 16); break; // WMADDH

        case 0x420B: // MDMAEN
            for (auto i = 0; i < 8; i++) {
                if (value & (1 << i)) // Each of the 8 bits signifies whether a DMA channel should fire a GPDMA
                    doGPDMA (i);
            }
            break;


        case 0x420D: Helpers::warn ("Unimplemented write to MEMSEL (val: {:02X})\n", value); break;

        case 0x4300 ... 0x430B: case 0x4310 ... 0x431B: case 0x4320 ... 0x432B: // DMA channel control registers
        case 0x4330 ... 0x433B: case 0x4340 ... 0x434B: case 0x4350 ... 0x435B:
        case 0x4360 ... 0x436B: case 0x4370 ... 0x437B: {
            const auto channel = (address >> 4) & 0xF;
            const auto reg = address & 0xF;

            dmaChannels[channel].controlRegs[reg] = value;
        } break;

        default: Helpers::warn ("Unimplemented write to system area address {:06X} (val: {:02X})\n", address, value); break;
    }
}

// Memory read function for the GUI's memory editor
static u8 Memory::read8Debugger (const u8* buffer, size_t address) {
    const auto page = address >> 11; // Divide address by 2048 to get the page
    const auto pointer = pageTableRead[page];

    if (pointer != nullptr) { // If this is a fast page, read directly
        const auto offset = address & 0x7FF; // Offset inside the page
        return pointer[offset];
    }

    else // This allows us to only read ROM/RAM for now
        return 0x69;
}

static void Memory::write8Debugger (u8* buffer, size_t address, u8 value) {
    const auto page = address >> 11; // Divide address by 2048 to get the page
    const auto pointer = pageTableWrite[page];

    if (pointer != nullptr) { // If this is a fast page, write directly
        const auto offset = address & 0x7FF; // Offset inside the page
        pointer[offset] = value;
    }

    else
        writeSlow <true> (address, value);
}