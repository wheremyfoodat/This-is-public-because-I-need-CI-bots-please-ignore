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
        cart.getROMInfo(dbEntry, directory);
    }

    mapFastmemPages(); // Fix our page tables so they fit with our new ROM
}

static void Memory::mapFastmemPages() {
    pageTableRead.fill (nullptr); // Erase page tables
    pageTableWrite.fill (nullptr);
    
    u32 size = cart.romSize * 1024;  // size of the ROM in bytes

    if (cart.mapper == Mappers::LoROM) { // Map LoROM
        if (cart.rom.size() >= 2 * megabyte) Helpers::panic ("LoROM ROM over 2MB");
        u32 romOffset = 0;

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

        // map LoROM SRAM to fastmem
        const auto sramSize = cart.ramSize * 1024;
        if (sramSize != 0 && cart.hasBattery) {
            if (Helpers::popcnt32(sramSize) != 1) // assert ram size is a power of 2
                Helpers::panic ("RAM size is not a power of 2!\n");

            auto sramMask = sramSize - 1;
            auto sramOffset = 0;

            for (auto page = 0xE00; page < 0xFB0;) { // Map SRAM pages as R/W
                pageTableRead[page] = &cart.sram[sramOffset];
                pageTableWrite[page] = &cart.sram[sramOffset];

                pageTableRead[page + 0x1000] = &cart.sram[sramOffset]; // Map upper SRAM as well
                pageTableWrite[page + 0x1000] = &cart.sram[sramOffset];

                sramOffset += pageSize;
                sramOffset &= sramMask; // Mirror SRAM if we go over the SRAM size

                page += ((page & 0xF) == 0xF) ? 33 : 1; // Skip to next SRAM bank if we're at the end of a 32KB SRAM bank
            }
        }
    }

    else if (cart.mapper == Mappers::HiROM) { // Map HiROM
        if (cart.rom.size() >= 4 * megabyte) Helpers::panic ("HiROM ROM over 4MB");
        u32 romOffset = 32 * kilobyte; // Top 32KB of the first bank

        // Map system area RAM and ROM to fastmem
        for (auto page = 0; page < 0x800;) { // Map LoROM banks
            for (auto i = 0; i < 4; i++) { // Map 4 2KB pages to system area RAM
                pageTableRead[page] = &wram[i * pageSize]; // Mark system area RAM as R/W
                pageTableWrite[page] = &wram[i * pageSize];
                    
                pageTableRead[page + 0x1000] = &wram[i * pageSize]; // Map the upper system area mirror as well
                pageTableWrite[page + 0x1000] = &wram[i * pageSize];

                page += 1; 
            }
 
            page += 12; // Skip 24KB ahead, to get to system area ROM
            
            for (auto i = 0; i < 16; i++) { // Map 16 ROM pages. In the LoROM area for HiROM carts, only the upper 32KB of each bank is mapped
                if (romOffset < size) { // Don't map pages if we've gone over the ROM size
                    pageTableRead[page] = &cart.rom[romOffset];
                    pageTableRead[page + 0x1000] = &cart.rom[romOffset]; // Mark the upper ROM mirror as well
                    romOffset += pageSize;
                }

                page += 1;
            }
            
            romOffset += 32 * kilobyte; // Skip next half-bank
        }

        romOffset = 0;
        for (auto page = 0x800; page < 0x1000; page++) { // map the HiROM ROM pages
            if (romOffset > size) break; // Stop mapping pages when we've gone over the size
            
            const auto pointer = &cart.rom[romOffset];
            if (page < 0xFC0) // WS1 HiROM is actually smaller than WS2 HiROM, as the last 2 banks are WRAM, so we map less memory to WS1 HiROM than WS2
                pageTableRead[page] = pointer;
            pageTableRead[page + 0x1000] = pointer; // Map WS2 HiROM
            
            romOffset += pageSize;
        }
    }

    else
        Helpers::panic ("Don't know how to map fastmem pages!!! Unknown mapper: {}\n", cart.mapperName());

    for (auto i = 0xFC0; i < 0x1000; i++) { // Map the 128KB of WRAM to page tables
        pageTableRead[i] = &wram[(i - 0xFC0) * pageSize];
        pageTableWrite[i] = &wram[(i - 0xFC0) * pageSize];
    }
}
static u8 sram[8192];

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
            case 0x2000 ... 0x2100: case 0x2200 ... 0x4000: Helpers::warn ("Read from unmapped memory (Addr: {:02X}:{:04X})\n", bank, address); return 0;
            case 0x6000 ... 0x7FFF: Helpers::warn ("Read from unimplemented expansion address: {:02X}:{:04X}\n", bank, address); return 0;
            case 0x436C: case 0x436D: Helpers::warn ("Read from whatever the fuck that was\n"); return 0;
            case 0x4B11: Helpers::warn ("Read from more weird invalid memory"); return 0;

            case 0x213F: Helpers::warn ("Read from PPU2 Status\n"); return 0;
            case 0x2140: case 0x2141: case 0x2142: case 0x2143: return rand(); // APU ports

            case 0x4210: { // rdnmi
                const auto val = ppu->rdnmi;
                ppu->rdnmi &= 0x7F; // Reading from rdnmi acknowledges the NMI and turns bit 7 off
                return val;
            }

            case 0x4211: { // timeup
                const auto val = ppu->timeup;
                ppu->timeup &= 0x7F; // Reading from timeup acknowledges the IRQ and turns bit 7 off
                return val;
            }

            case 0x4212: // hvbjoy
                ppu->hvbjoy ^= 1; // We're stubbing the low bit
                return ppu->hvbjoy;

            // Math engine registers
            case 0x4214: return mathEngine.quotient & 0xFF; 
            case 0x4215: return mathEngine.quotient >> 8;
            case 0x4216: return mathEngine.division_remainder_multiplication_product & 0xFF;
            case 0x4217: return mathEngine.division_remainder_multiplication_product >> 8;
                
            // Automatic reading joypad ports
            case 0x4218: return Joypads::pad1 & 0xFF; // Joypad 1 (Low) 
            case 0x4219: return Joypads::pad1 >> 8; // Joypad 1 (high)
            case 0x421A: case 0x421B: return 0; // Joypad 2 (Unimplemented)

            // Manual reading joypad ports
            case 0x4016: case 0x4017: return 0; // Joypad Input Register A and B (unimplemented)

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

        case 0x210D: // BG1HOFS / M7HOFS // TODO: Mode 7
            ppu->hofs[0] = (value << 8) | (ppu->old_hofs[0] & ~7) | ((ppu->hofs[0] >> 8) & 7);
            ppu->old_hofs[0] = value;
            break;

        case 0x210E: // BG1VOFS / M7VOFS // TODO: Mode 7
            ppu->vofs[0] = (value << 8) | ppu->old_vofs[0];
            ppu->old_vofs[0] = value;
            break;

        case 0x210F: // BG2HOFS
            ppu->hofs[1] = (value << 8) | (ppu->old_hofs[1] & ~7) | ((ppu->hofs[1] >> 8) & 7);
            ppu->old_hofs[1] = value;
            break;

        case 0x2110: // BG2VOFS
            ppu->vofs[1] = (value << 8) | ppu->old_vofs[1];
            ppu->old_vofs[1] = value;
            break;

        case 0x2111: // BG3HOFS
            ppu->hofs[2] = (value << 8) | (ppu->old_hofs[2] & ~7) | ((ppu->hofs[2] >> 8) & 7);
            ppu->old_hofs[2] = value;
            break;

        case 0x2112: // BG3VOFS
            ppu->vofs[2] = (value << 8) | ppu->old_vofs[2];
            ppu->old_vofs[2] = value;
            break;

        case 0x2113: // BG4HOFS
            ppu->hofs[3] = (value << 8) | (ppu->old_hofs[3] & ~7) | ((ppu->hofs[3] >> 8) & 7);
            ppu->old_hofs[3] = value;
            break;

        case 0x2114: // BG4VOFS
            ppu->vofs[3] = (value << 8) | ppu->old_vofs[3];
            ppu->old_vofs[3] = value;
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

                const auto red = Helpers::get8BitColor (palette & 0x1F); // Convert palette to RGBA8888 and cache it for later to be used by the PPU
                const auto green = Helpers::get8BitColor ((palette >> 5) & 0x1F);
                const auto blue = Helpers::get8BitColor ((palette >> 10) & 0x1F);
                ppu->paletteCache[ppu->paletteAddr] = 0xFF000000 | red | (green << 8) | (blue << 16);

                ppu->paletteAddr++; // Increment palette address
            }

            else
                ppu->latchedPalette = value;

            ppu->paletteLatch = !ppu->paletteLatch;
        } break;

        case 0x212C: ppu->tm = value; break;

        case 0x2180: // WMDATA
            wram[wramAddress++] = value;
            wramAddress &= 0x1FFFF;
            break;

        case 0x2181: wramAddress = (wramAddress & ~0xFF) | value; break; // WMADDL
        case 0x2182: wramAddress = (wramAddress & ~0xFF00) | (value << 8); break; // WMADDM;
        case 0x2183: wramAddress = (wramAddress & 0xFFFF) | ((value & 1) << 16); break; // WMADDH

        case 0x4200: { // NMITIMEN
            const bool nmiRisingEdge = !(ppu->nmitimen & 0x80) && (value & 0x80); // Check if the NMI enable flag went from 0 to 1
            ppu->nmitimen = value; 
            if (value & 0x30) 
                Helpers::warn ("Enabled H/V IRQs\n");

            if (nmiRisingEdge && (ppu->rdnmi & 0x80)) // Check if NMIs just got enabled and were already requested, and fire an NMI if so
                scheduler->pushEvent (EventTypes::FireNMI, 0); // Timestamp = 0 means it will instantly get executed
        } break; 

        case 0x4202: mathEngine.multiplicand = value; break; // WRMPYA
        case 0x4203: // WRMPYB
            mathEngine.multiplier = value;
            mathEngine.division_remainder_multiplication_product = (u16) mathEngine.multiplicand * (u16) mathEngine.multiplier;
            break; 

        case 0x4204: mathEngine.dividend = (mathEngine.dividend & 0xFF00) | value; break; // WRDIVL
        case 0x4205: mathEngine.dividend = (mathEngine.dividend & 0x00FF) | (value << 8); break; // WRDIVH
        case 0x4206: // WRDIVB
            mathEngine.divisor = value;
            mathEngine.quotient = (value) ? 0xFFFF : (u16) mathEngine.dividend / (u16) mathEngine.divisor; // Quotient is 0xFFFF is divisor is 0, else it's dividend/divisor
            // Similarly for remainder, it's equal to the dividend if divisor == 0, else it's equal to the dividend % divisor
            mathEngine.division_remainder_multiplication_product = (value) ? mathEngine.dividend : (u16) mathEngine.dividend % (u16) mathEngine.divisor;
            break;

        case 0x420B: // MDMAEN
            for (auto i = 0; i < 8; i++) {
                if (value & (1 << i)) // Each of the 8 bits signifies whether a DMA channel should fire a GPDMA
                    doGPDMA (i);
            }
            break;
        
        case 0x420C: if (value) Helpers::warn ("Fired HDMA (HDMAEN: {:02X})", value); break;

        case 0x420D: Helpers::warn ("Unimplemented write to MEMSEL (val: {:02X})\n", value); break;

        case 0x4300 ... 0x430B: case 0x4310 ... 0x431B: case 0x4320 ... 0x432B: // DMA channel control registers
        case 0x4330 ... 0x433B: case 0x4340 ... 0x434B: case 0x4350 ... 0x435B:
        case 0x4360 ... 0x436B: case 0x4370 ... 0x437B: {
            const auto channel = (address >> 4) & 0xF;
            const auto reg = address & 0xF;

            dmaChannels[channel].controlRegs[reg] = value;
        } break;

        default: return; Helpers::warn ("Unimplemented write to system area address {:06X} (val: {:02X})\n", address, value); break;
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