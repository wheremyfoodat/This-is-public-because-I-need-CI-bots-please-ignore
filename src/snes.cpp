#include "snes.hpp"

// Set up game databases
SNES::SNES() {
    const auto dbPath = std::filesystem::current_path() / "snes_db.json";

    std::ifstream db(dbPath);
    if (db.fail())
        Helpers::panic ("No game database exists! Please use the provided snes_db.json\n");

    db >> Memory::gameDB;
    Memory::ppu = &ppu;
    Memory::scheduler = &scheduler;
}

void SNES::reset() { // TODO: Reset APU, PPU, scheduler, etc
    cpu.reset();
    Memory::apu = SPC700();
}

void SNES::runFrame() {
    while (!frameDone)
        step();

    frameDone = false;
}

void SNES::step() {
    cpu.step();
    scheduler.addCycles (cpu.cycles * 6); // Assume 1 CPU cycle = 6 master clock cycles (This depends on memory waitstates, we're assuming we're always running @3.58MHz)

    while (true) {
        const auto e = scheduler.next();
        if (scheduler.timestamp >= e.timestamp) { // Check if any events should be fired
            scheduler.removeNext();
            switch (e.type) {
                case EventTypes::HBlank:
                    if (ppu.line < 224)
                        ppu.renderScanline();
                    ppu.hvbjoy |= 0x40; // Set HBlank flag in HVBJoy
                    scheduler.pushEvent (EventTypes::EndOfLine, e.timestamp + 258); // Schedule end of line event
                    break;

                case EventTypes::EndOfLine:
                    ppu.line += 1; // Increment PPU line counter
                    ppu.hvbjoy &= ~0x40; // Turn off H-Blank flag in hvbjoy

                    if (ppu.line == 224) { // Check if we just entered vblank
                        frameDone = true; // We can go back to the frontend real quick
                        ppu.rdnmi |= 0x80; // Request VBlank NMI
                        ppu.hvbjoy |= 0x80; // Turn on V-Blank flag in hvbjoy

                        if (ppu.nmitimen & 0x80) // Fire NMI if they're enabled
                            cpu.fireNMI();
                    }
 
                    else if (ppu.line == 262) { // Check if we're leaving vblank
                        ppu.line = 0;
                        ppu.rdnmi &= 0x7F; // Remove VBlank NMI request
                        ppu.hvbjoy &= 0x7F; // Turn off V-Blank flag in hvbjoy
                    }

                    scheduler.pushEvent (EventTypes::HBlank, e.timestamp + 1106); // Schedule next HBlank
                    break;
                    
                case EventTypes::FireNMI: cpu.fireNMI(); break;

                default: Helpers::panic ("Unhandled event: {}\n", e.name());
            }
        }

        else 
            break;
    }
}