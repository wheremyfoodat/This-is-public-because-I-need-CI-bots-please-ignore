#include "APU/spc700.hpp"
#include "utils.hpp"

u8 SPC700::read (u16 address) {
    if (address >= 0xF0 && address <= 0xFF) { // Hande IO ports
        switch (address) {
            case 0xF0: case 0xF1: return 0; // Write-only
            case 0xF4: return inputPorts[0];
            case 0xF5: return inputPorts[1];
            case 0xF6: return inputPorts[2];
            case 0xF7: return inputPorts[3];

            case 0xFD: // Timer 0 output
                timer0.update (cycles); // Lazily update it
                return timer0.read();

            case 0xFE: // Timer 1 output
                timer1.update (cycles); // Lazily update it
                return timer1.read();

            case 0xFF: // Timer 2 output
                timer2.update (cycles); // Lazily update it
                return timer2.read();

            default: Helpers::panic ("[SPC700] Read from unknown IO port {:02X}\n", address);
        }
    }

    else if (address >= 0xFFC0) // This address space is either taken up by the bootrom or ram, depending on IO port F1.7
        return bootromMapped ? bootrom[address & 0x3F] : ram[address];

    else return ram[address];
}

u16 SPC700::read16 (u16 address) {
    return read (address) | (read (address + 1) << 8);
}

void SPC700::write (u16 address, u8 value) {
    if (address >= 0xF0 && address <= 0xFF) { // Handle IO ports
        switch (address) {
            case 0xF1: // CONTROL register
                bootromMapped = (value & 0x80) != 0; // Bit 7 of control tells us whether to map the bootrom or not
                if (!(value & 0x1)) timer0.disable (cycles); // Bits 0-2 enable or disable the timers
                else if (!timer0.enabled) timer0.enable (cycles); // Check if we went from disabled to enabled

                if (!(value & 0x2)) timer1.disable (cycles);
                else if (!timer1.enabled) timer1.enable (cycles); // Check if we went from disabled to enabled

                if (!(value & 0x4)) timer2.disable (cycles);
                else if (!timer2.enabled) timer2.enable (cycles); // Check if we went from disabled to enabled

                if (value & 0x10) { // Writing 1 to bit 4 resets the CPU->APU input ports 0 and 1 to 0
                    inputPorts[0] = 0;
                    inputPorts[1] = 0;
                }

                if (value & 0x20) { // Writing 1 to bit 5 resets the CPU->APU input ports 2 and 3 to 0
                    inputPorts[2] = 0;
                    inputPorts[3] = 0;
                }

                break;

            case 0xF2: dspRegisterIndex = value; break;
            case 0xF3: break; // DSP register data. We currently don't emulate the DSP :(
            case 0xF4: outputPorts[0] = value; break;
            case 0xF5: outputPorts[1] = value; break;
            case 0xF6: outputPorts[2] = value; break;
            case 0xF7: outputPorts[3] = value; break;
            case 0xFA: // Timer 0 divider
                timer0.update (cycles);
                timer0.divider = value ? value : 256; // A divider of 0 means 256
                break;

            case 0xFB: // Timer 1 divider
                timer1.update (cycles);
                timer1.divider = value ? value : 256; // A divider of 0 means 256
                break;

            case 0xFC: // Timer 2 divider
                timer2.update (cycles);
                timer2.divider = value ? value : 256; // A divider of 0 means 256
                break;

            default: Helpers::panic ("[SPC700] Wrote to unknown IO port {:02X}(Val: {:02X})\n", address, value);
        }
    }

    else ram[address] = value;
}