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
}