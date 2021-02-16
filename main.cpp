#include <iostream>
#include "chip8.hpp"

int main(int argc, char** argv) {
    chip8::Chip8 emulator("roms/spaceInvaders.ch8");
    emulator.emulate();

    return 0;
}