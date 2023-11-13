#include "ultra64.h"
#include "ultramodern.hpp"

void ultramodern::preinit(uint8_t* rdram, uint8_t* rom, ultramodern::WindowHandle window_handle) {
    ultramodern::set_main_thread();
    ultramodern::init_events(rdram, rom, window_handle);
    ultramodern::init_timers(rdram);
    ultramodern::init_audio();
    ultramodern::save_init();
}

extern "C" void osInitialize() {
    ultramodern::init_scheduler();
}
