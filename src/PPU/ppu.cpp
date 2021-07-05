#include "PPU/ppu.hpp"

// Stub
// Renders a scanline of 2bpp BG0 
void PPU::renderScanline() {
    const auto bgSize = sc[0].size;
    const auto ypos = line; // TODO: HOFS

    auto tileDataStart = nba[0] << 12; // Multiply base address by 4096 K-Words
    auto bgMapStart = (u32) sc[0].base;

    bgMapStart &= 0x1F; // Mask out top bit
    bgMapStart <<= 10; // Multiply by 1024 K-Words

    if (bgSize) Helpers::panic ("BG with non-0 size\n");

    auto frameBufferIndex = line * 256 * 4; // The screen is 256 pixels wide, each pixel being 4 bytes
    const auto tileY = ypos & 7; // Which line of the tile are we in?

    for (auto x = 0; x < 256; x++) {
        const auto tileX = x & 7; // Which column of the tile are we in?
        const auto tileMapAddr = ((ypos >> 3) << 5) + (x >> 3) + bgMapStart;

        const auto mapEntry = vram[tileMapAddr]; // Fetch the tile map entry
        const auto tileNum = mapEntry & 0x3FF;
        const auto palNum = (mapEntry >> 10) & 7;

        const auto tileAddr = tileDataStart + tileNum * 8; // Address of the tile to render
        const auto lineAddr = tileAddr + tileY; // Address of the line of the tile to render

        const auto line = vram[lineAddr];
        const auto lineLow = line & 0xFF;
        const auto lineHigh = line >> 8;

        // Use the low and high bytes to get the palette index from the bit-plane
        // The "x ^ 7" is just a faster way of doing "7-x" provided x is in range [0, 7]
        const auto palIndex = ((lineLow >> (tileX ^ 7) & 1)) | ((lineHigh >> (tileX ^ 7) & 1) << 1);
        const auto palette = paletteRAM[palIndex + 4 * palNum];

        const auto blue = (palette >> 10) & 0x1F;
        const auto green = (palette >> 5) & 0x1F;
        const auto red = palette & 0x1F;

        framebuffer[frameBufferIndex] = Helpers::get8BitColor(red);
        framebuffer[frameBufferIndex+1] = Helpers::get8BitColor(green);
        framebuffer[frameBufferIndex+2] = Helpers::get8BitColor(blue);

        frameBufferIndex += 4;
    }
}