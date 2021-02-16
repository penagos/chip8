#include <iostream>
#include "chip8.hpp"

int main(int argc, char** argv) {
    chip8::Chip8 emulator(argv[1]);
    emulator.emulate();

    return 0;
}