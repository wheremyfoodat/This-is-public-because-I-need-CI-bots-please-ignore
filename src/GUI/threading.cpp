#include "GUI/gui.hpp"
#include "snes.hpp"

// Run our SNES instance on another thread.
void SNES::runAsync() {
    while (true) {
        waitPing(); // Sleep until the main thread tells us to run a frame
        runFrame(); // Once it tells us to run a frame, run a frame
        run_emu_thread = false; // Tell the GUI thread we're done running
    }
}

// Makes the emulator thread wait for the GUI thread to send a signal (via run_emu_thread) to run a new frame
void SNES::waitPing() {
    std::unique_lock <std::mutex> lock (emu_mutex);
    emu_condition_variable.wait(lock, [&]{ return run_emu_thread == true; }); // slep until the GUI tells us to wake up
}

// Wake up the SNES thread, tell it to run for a frame while the GUI thread is doing GUI stuff
void GUI::pingEmuThread() {
    std::lock_guard <std::mutex> lock (g_snes.emu_mutex); // Get ready to send the signal
    
    g_snes.run_emu_thread = true; // Tell the SNES thread it's time to start running
    g_snes.emu_condition_variable.notify_one(); // Ping the SNES thread's conditional variable to tell it to wake up
}

// Make the GUI thread wait until the SNES thread is done running a frame
void GUI::waitEmuThread() {
    while (g_snes.run_emu_thread) {}
}