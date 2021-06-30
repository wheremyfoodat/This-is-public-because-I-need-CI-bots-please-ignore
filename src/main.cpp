#include "snes.hpp"
#include "gui.hpp"

int main() {
    auto gui = GUI();

    while (gui.isOpen()) {
        gui.update();
    }

    ImGui::SFML::Shutdown();
}
