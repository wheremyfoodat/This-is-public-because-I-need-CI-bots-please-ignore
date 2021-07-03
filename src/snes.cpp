#include "snes.hpp"

// Set up game databases
SNES::SNES() {
    const auto dbPath = std::filesystem::current_path() / "snes_db.json";

    std::ifstream db(dbPath);
    if (db.fail())
        Helpers::panic ("No game database exists! Please use the provided snes_db.json\n");

    db >> Memory::gameDB;
}

void SNES::reset() {
    cpu.reset();
}

void SNES::runFrame() {
    while (true) { // TODO: Make this actually run a frame and not until the heat death of the universe
        cpu.step();
    }
}

void SNES::step() {
    cpu.step();
}