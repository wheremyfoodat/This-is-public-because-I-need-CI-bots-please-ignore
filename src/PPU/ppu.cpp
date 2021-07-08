#include "PPU/ppu.hpp"

// Stub
// Renders a scanline of 2bpp BG0 
template <Depth depth, int number>
void PPU::renderBG() {
    constexpr int index = number - 1; // BGs are numbered 1-4 but array indices start at 0
    auto framebuffer = buffers[bufferIndex];

    const auto bgSize = sc[index].size;
    unsigned ypos = line + vofs[index]; // TODO: VOFS
    unsigned xpos = hofs[index]; // TODO: HOFS here

    auto tileDataStart = nba[index] << 12; // Multiply base address by 4096 K-Words
    auto bgMapStart = (u32) sc[index].base;

    bgMapStart &= 0x1F; // Mask out top bit
    bgMapStart <<= 10; // Multiply by 1024 K-Words

    if ((ypos & 0x1FF) > 255) {
        switch (bgSize) {
            case 0: case 1: break; // These are 32-tall, so we don't do anything
            case 2: bgMapStart += 0x400; break; // Handle 64-tile-tall background sizes
            case 3: bgMapStart += 0x800; break; // On 64x64 BG maps, we need to offset the BG map by 0x800 instead of 0x400, because the tile map is 64 tiles wide instead of 32
        }
    }

    auto frameBufferIndex = line * 256 * 4; // The screen is 256 pixels wide, each pixel being 4 bytes
    const auto tileY = ypos & 7; // Which line of the tile are we in?

    // The tile info
    unsigned palNum;
    unsigned line1_low, line1_high, line2_low, line2_high, line3_low, line3_high, line4_low, line4_high;

    for (auto x = 0; x < 256; x++) {
        const auto tileX = xpos & 7; // Which column of the tile are we in?
        
        if (x == 0 || tileX == 0) { // Fetch the information for a new tile if we need to (IE if we got to the end of a tile, or if we just started rendering)
            auto tileMapAddr = (((ypos >> 3) & 31) << 5) + ((u8) xpos >> 3) + bgMapStart; // >> 3: Divide ypos by 8 to see which row on the tilemap we're at
                                                                                           // & 31: Mask the row number by 31 - After all, the tile map is simply 1 or more 32x32 maps, depending on BGSIZE
                                                                                           // << 5: Multiply by 32 to get the address of the tilemap row
                                                                                           // + ((u8) xpos >> 3): Index into the tile row to get the tilemap entry address, masking xpos like we mask ypos
            if ((xpos & 0x1FF) > 255) { // Handle 64-wide BGs
                if (bgSize == 1 || bgSize == 3)
                    tileMapAddr += 0x400;
            }
            
            const auto mapEntry = vram[tileMapAddr]; // Fetch the tile map entry
            const auto tileNum = mapEntry & 0x3FF;
            palNum = (mapEntry >> 10) & 7;

            if constexpr (depth == Depth::Bpp2) {
                const u32 tileAddr = tileDataStart + tileNum * 8;
                const auto lineAddr = tileAddr + tileY; // Address of the line of the tile to render

                const auto line = vram[lineAddr];
                line1_low = line & 0xFF;
                line1_high = line >> 8;
            }

            else if constexpr (depth == Depth::Bpp4) {
                const u32 tileAddr = tileDataStart + tileNum * 16;
                const auto lineAddr = tileAddr + tileY; // Address of the line of the tile to render

                const auto line1 = vram[lineAddr];
                const auto line2 = vram[lineAddr + 8];

                line1_low = line1 & 0xFF;
                line1_high = line1 >> 8;
                line2_low = line2 & 0xFF;
                line2_high = line2 >> 8;
            }

            else {
                const u32 tileAddr = tileDataStart + tileNum * 32;
                const auto lineAddr = tileAddr + tileY; // Address of the line of the tile to render

                const auto line1 = vram[lineAddr];
                const auto line2 = vram[lineAddr + 8];
                const auto line3 = vram[lineAddr + 16];
                const auto line4 = vram[lineAddr + 24];

                line1_low = line1 & 0xFF;
                line1_high = line1 >> 8;
                line2_low = line2 & 0xFF;
                line2_high = line2 >> 8;
                line3_low = line3 & 0xFF;
                line3_high = line3 >> 8;
                line4_low = line4 & 0xFF;
                line4_high = line4 >> 8;
            }
        }

        u32 palette;
        
        // Use the low and high bytes to get the palette index from the bit-plane
        // The "x ^ 7" is just a faster way of doing "7-x" provided x is in range [0, 7]
        if constexpr (depth == Depth::Bpp2) {
            const auto palIndex = ((line1_low >> (tileX ^ 7) & 1)) | ((line1_high >> (tileX ^ 7) & 1) << 1);
            palette = paletteCache[palIndex + 4 * palNum];
        }

        // Same here, except with 4 bits per pixel instead
        else if constexpr (depth == Depth::Bpp4) {
            const auto palIndex = ((line1_low >> (tileX ^ 7) & 1)) | ((line1_high >> (tileX ^ 7) & 1) << 1) | ((line2_low >> (tileX ^ 7) & 1) << 2) | ((line2_high >> (tileX ^ 7) & 1) << 3);
            palette = paletteCache[palIndex + 16 * palNum];
        }

        else {
            const auto palIndex = ((line1_low >> (tileX ^ 7) & 1)) | ((line1_high >> (tileX ^ 7) & 1) << 1) | ((line2_low >> (tileX ^ 7) & 1) << 2) | ((line2_high >> (tileX ^ 7) & 1) << 3) |
                                  ((line3_low >> (tileX ^ 7) & 1) << 4) | ((line3_high >> (tileX ^ 7) & 1) << 5) | ((line4_low >> (tileX ^ 7) & 1) << 6) | ((line4_high >> (tileX ^ 7) & 1) << 7);
            palette = paletteCache[palIndex];
        }

        *(u32*) &framebuffer[frameBufferIndex] = palette;
        frameBufferIndex += 4;
        xpos++;
    }
}

void PPU::renderScanline() {
    switch (bgmode.mode) { // TODO: Implement all BG modes properly
        case 0: renderBG <Depth::Bpp2, 1>(); break;
        case 1: renderBG <Depth::Bpp2, 3>(); break;
        case 6: renderBG <Depth::Bpp4, 1>(); break;
        default: Helpers::panic ("Unimplemented BG mode {}\n", bgmode.mode);
    }
}