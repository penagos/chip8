#include <random>
#include <string>

#include "common.hpp"
#include "op.hpp"

namespace chip8 {
class Chip8 final {
    public:
        Chip8(std::string aROMName);
        void emulate();
    private:
        void boot();
        void loadROM(std::string aROMName);
        void error(std::string aMessage) const;

        // Debugging facilities
        void dumpMemory() const;

        // CHIP-8 opcodes are 2 bytes long
        uint16_t opcode;

        // CHIP-8 has a maximum memory space of 4K bytes. Historically the first 512
        // bytes of memory (0x200) were reserved for the emulator. Uppermost 256 bytes
        // are reserved for display refresh. Following 96 bytes usually reserved for
        // call stack and other internal use
       uint8_t memory[MAX_MEM];

        // ROM files expect fonts to be installed at specific locations in memory
        // Use the fonts array below at boot time to load font sprites
        uint8_t fonts[FONTS_SIZE] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

        // CHIP-8 has 16 b-bit registers V0-VF. The VF register usually stores (carry)
        // flags and should not be used as a general purpose register
        uint8_t V[NUM_REGS];

        // Address register is 16 bits wide
        uint16_t I;
        uint16_t pc;
        uint16_t sp;

        // Original CHIP-8 stack size was 8 bytes
        uint8_t stack[STACK_SIZE];

        // CHIP-8 graphics screen is 64x32 pixels where sprites are XORed
        uint8_t gfx[GFX_WIDTH * GFX_HEIGHT];

        // Timers used for events and sounds
        uint8_t delayTimer;
        uint8_t sndTimer;

        // Map 16 input keys to a state array
        uint8_t key[MAX_KEYS];

        // Some instructions in the CHIP-8 ISA rely on a random number value.
        // In hardware this is usually accomplished with a dedicated chip or
        // reading a noisy signal
        std::uniform_int_distribution<uint8_t> rand;
        std::default_random_engine randGen;

        // Below are the 35 instructions defined by the CHIP-8 ISA
        void _0nnn();
        void _00e0();
        void _00ee();
        void _1nnn();
        void _2nnn();
};
} // chip8 namespace