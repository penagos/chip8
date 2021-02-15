#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

#include "chip8.hpp"

namespace chip8 {
Chip8::Chip8(std::string aROMName)
    : pc(MEM_LO)
    , randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    boot();
    loadROM(aROMName);
}

void Chip8::boot() {
    // Mostly sanity checks
    std::cout << "Booting with " << MAX_MEM << " bytes of memory (usable = "
              << MEM_HI - MEM_LO << " bytes)" << std::endl;

    // Load fonts into memory
    std::copy(std::begin(fonts), std::end(fonts), (std::begin(memory) + MEM_FNT));

    // Initialize rand register
    rand = std::uniform_int_distribution<uint8_t>(0, 255U);
}

void Chip8::loadROM(std::string aROMName) {
    if (aROMName.empty()) {
        error("ROM filename cannot be empty");
    }

    std::ifstream rom(aROMName, std::ios::binary | std::ios::ate);

    if (!rom.is_open()) {
        error("Failed to open ROM file");
    }

    std::streampos size = rom.tellg();

    if (size > MEM_HI - MEM_LO) {
        error("ROM file exceeds usable memory");
    }

    rom.seekg(0, std::ios::beg);
    rom.read((((char*)memory) + MEM_LO), size);
    rom.close();
}

void Chip8::emulate() {
    while (true) {

    }
}

void Chip8::dumpMemory() const {

}

void Chip8::error(std::string aMessage) const {
    std::cerr << aMessage << std::endl;
    exit(1);
}

// Instructions defined below

// 0nnn - SYS addr
// Jump to a machine code routine at nnn
void Chip8::_0nnn() {

}

// 00E0 - CLS
// Clear the display
void Chip8::_00e0() {
    memset((char*)gfx, 0, sizeof(gfx));
}

// 00EE - RET
// Return from a subroutine
void Chip8::_00ee() {
    // Decrement stack pointer, pop the stack and set PC to instruction at the
    // top of the newly popped stack
    pc = stack[--sp];
}

// 1nnn - JP addr
// Jump to location nnn
void Chip8::_1nnn() {
    pc = opcode & 0x0FFFu;
}

// 2nnn - CALL addr
// Call subroutine at nnn
void Chip8::_2nnn() {
    stack[sp++] = pc;
    pc = opcode & 0x0FFFu;
}

} // chip8 namespace