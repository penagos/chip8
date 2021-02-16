#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

#include "chip8.hpp"

namespace chip8 {
Chip8::Chip8(std::string aROMName)
    : pc(MEM_LO)
    , randGen(std::chrono::system_clock::now().time_since_epoch().count())
    , quit(false) {
    boot();
    loadInsts();
    loadROM(aROMName);
}

Chip8::~Chip8() {
    delete gfxHandle;
}

void Chip8::boot() {
    // Mostly sanity checks
    std::cout << "Booting with " << MAX_MEM << " bytes of memory (usable = "
              << MEM_HI - MEM_LO << " bytes)" << std::endl;

    // Load fonts into memory
    std::copy(std::begin(fonts), std::end(fonts), (std::begin(memory) + MEM_FNT));

    // Initialize rand register
    rand = std::uniform_int_distribution<uint8_t>(0, 255U);

    pc = MEM_LO;

    int videoScale = std::stoi("10");

    gfxHandle = new Gfx("CHIP-8 Emulator", GFX_WIDTH * videoScale, GFX_HEIGHT * videoScale,
                        GFX_WIDTH, GFX_HEIGHT);
}

void Chip8::loadInsts() {
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::_1nnn;
    table[0x2] = &Chip8::_2nnn;
    table[0x3] = &Chip8::_3xkk;
    table[0x4] = &Chip8::_4xkk;
    table[0x5] = &Chip8::_5xy0;
    table[0x6] = &Chip8::_6xkk;
    table[0x7] = &Chip8::_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::_9xy0;
    table[0xA] = &Chip8::_annn;
    table[0xB] = &Chip8::_bnnn;
    table[0xC] = &Chip8::_cxkk;
    table[0xD] = &Chip8::_dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    table0[0x0] = &Chip8::_00e0;
    table0[0xE] = &Chip8::_00ee;

    table8[0x0] = &Chip8::_8xy0;
    table8[0x1] = &Chip8::_8xy1;
    table8[0x2] = &Chip8::_8xy2;
    table8[0x3] = &Chip8::_8xy3;
    table8[0x4] = &Chip8::_8xy4;
    table8[0x5] = &Chip8::_8xy5;
    table8[0x6] = &Chip8::_8xy6;
    table8[0x7] = &Chip8::_8xy7;
    table8[0xE] = &Chip8::_8xyE;

    tableE[0x1] = &Chip8::_exa1;
    tableE[0xE] = &Chip8::_ex9e;

    tableF[0x07] = &Chip8::_fx07;
    tableF[0x0A] = &Chip8::_fx0a;
    tableF[0x15] = &Chip8::_fx15;
    tableF[0x18] = &Chip8::_fx18;
    tableF[0x1E] = &Chip8::_fx1e;
    tableF[0x29] = &Chip8::_fx29;
    tableF[0x33] = &Chip8::_fx33;
    tableF[0x55] = &Chip8::_fx55;
    tableF[0x65] = &Chip8::_fx65;
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
    int videoPitch = sizeof(gfx[0]) * GFX_WIDTH;
	auto lastCycleTime = std::chrono::high_resolution_clock::now();
    int cycleDelay = std::stoi("1");

    while (!quit) {
        quit = gfxHandle->input(key);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay) {
			lastCycleTime = currentTime;
            tick();
            gfxHandle->update(gfx, videoPitch);
        }
    }
}

// Simulate 1 clock tick
void Chip8::tick() {
    // Fetch
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	// Increment the PC before we execute anything
	pc += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0) {
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (sndTimer > 0) {
		--sndTimer;
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

// 3xkk - SE Vx, byte
// Skip next instruction if Vx = kk
void Chip8::_3xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

    if (V[Vx] == byte) {
        pc += 2;
    }
}

// 4xkk - SNE Vx, byte
// Skip next instruction if Vx != kk
void Chip8::_4xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

    if (V[Vx] != byte) {
        pc += 2;
    }
}

// 5xy0 - SE Vx, Vy
// Skip next instruction if Vx = Vy
void Chip8::_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (V[Vx] == V[Vy]) {
		pc += 2;
	}
}

// 6xkk - LD Vx, byte
// Set Vx = kk
void Chip8::_6xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[Vx] = byte;
}

// 7xkk - ADD Vx, byte
// Vx = Vx + kk
void Chip8::_7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[Vx] += byte;
}

// 8xy0 - LD Vx, Vy
// Set Vx = Vy
void Chip8::_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	V[Vx] = V[Vy];
}

// 8xy1 - OR Vx, Vy
// Set Vx OR Vy
void Chip8::_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	V[Vx] |= V[Vy];
}

// 8xy2 - AND Vx, Vy
// Set Vx AND Vy
void Chip8::_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	V[Vx] &= V[Vy];
}

// 8xy2 - XOR Vx, Vy
// Set Vx XOR Vy
void Chip8::_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	V[Vx] ^= V[Vy];
}

// 8xy4 - ADD Vx, Vy
// Set Vx = Vx + Vy, set VF = carry
void Chip8::_8xy4() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = V[Vx] + V[Vy];

	if (sum > 255U) {
		V[0xF] = 1;
	} else {
		V[0xF] = 0;
	}

	V[Vx] = sum & 0xFFu;
}

// 8xy5 - SUB Vx, Vy
// Set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::_8xy5() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (V[Vx] > V[Vy]) {
		V[0xF] = 1;
	} else {
		V[0xF] = 0;
	}

	V[Vx] -= V[Vy];
}

// 8xy6 - SHR Vx {, Vy}
// Set Vx = Vx SHR 1
void Chip8::_8xy6() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save LSB in VF
	V[0xF] = (V[Vx] & 0x1u);

	V[Vx] >>= 1;
}

// 8xy7 - SUBN Vx, Vy
// Set Vx = Vy - Vx, set VF = NOT borrow
void Chip8::_8xy7() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (V[Vy] > V[Vx]) {
		V[0xF] = 1;
	} else {
		V[0xF] = 0;
	}

	V[Vx] = V[Vy] - V[Vx];
}

// 8xyE - SHL Vx {, Vy}
// Set Vx = Vx SHL 1
void Chip8::_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	V[0xF] = (V[Vx] & 0x80u) >> 7u;

	V[Vx] <<= 1;
}

// 9xy0 - SNE Vx, Vy
// Skip next instruction if Vx != Vy
void Chip8::_9xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (V[Vx] != V[Vy]) {
		pc += 2;
	}
}

// Annn - LD I, addr
// Set I = nnn
void Chip8::_annn() {
    uint16_t address = opcode & 0x0FFFu;
	I = address;
}

// Bnnn - JP V0, addr
// Jump to location nnn + V0
void Chip8::_bnnn() {
    uint16_t address = opcode & 0x0FFFu;
	pc = V[0] + address;
}

// Cxkk - RND Vx, byte
// Set Vx = random byte AND kk
void Chip8::_cxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[Vx] = rand(randGen) & byte;
}

// Dxyn - DRW Vx, Vy, nibble
// Display n-byte sprite starting at memory location I at (Vx, Vy),
// set VF = collision
void Chip8::_dxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = V[Vx] % GFX_WIDTH;
	uint8_t yPos = V[Vy] % GFX_HEIGHT;

	V[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[I + row];

		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint8_t* screenPixel = &gfx[(yPos + row) * GFX_WIDTH + (xPos+col)];

			// Sprite pixel is on
			if (spritePixel) {
				// Screen pixel also on - collision
				if (*screenPixel == 0xFF) {
					V[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFF;
			}
		}
	}
}

// Ex9E - SKP Vx
// Skip next instruction if key with the value of Vx is pressed
void Chip8::_ex9e() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t k = V[Vx];

	if (key[k]) {
		pc += 2;
	}
}

// ExA1 - SKNP Vx
// Skip next instruction if key with the value of Vx is not pressed
void Chip8::_exa1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t k = V[Vx];

	if (!key[k]) {
		pc += 2;
	}
}

// Fx07 - LD Vx, DT
// Set Vx = delay timer value
void Chip8::_fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	V[Vx] = delayTimer;
}

// Fx0A - LD Vx, K
// Wait for a key press, store the value of the key in Vx
void Chip8::_fx0a() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (key[0]) {
		V[Vx] = 0;
	} else if (key[1]) {
		V[Vx] = 1;
	} else if (key[2]) {
		V[Vx] = 2;
	} else if (key[3]) {
		V[Vx] = 3;
	} else if (key[4]) {
		V[Vx] = 4;
	} else if (key[5]) {
		V[Vx] = 5;
	} else if (key[6]) {
		V[Vx] = 6;
	} else if (key[7]) {
		V[Vx] = 7;
	} else if (key[8]) {
		V[Vx] = 8;
	} else if (key[9]) {
		V[Vx] = 9;
	} else if (key[10]) {
		V[Vx] = 10;
	} else if (key[11]) {
		V[Vx] = 11;
	} else if (key[12]) {
		V[Vx] = 12;
	} else if (key[13]) {
		V[Vx] = 13;
	} else if (key[14]) {
		V[Vx] = 14;
	} else if (key[15]) {
		V[Vx] = 15;
	} else {
		pc -= 2;
	}
}

// Fx15 - LD DT, Vx
// Set delay timer = Vx
void Chip8::_fx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	delayTimer = V[Vx];
}

// Fx18 - LD ST, Vx
// Set sound timer = Vx
void Chip8::_fx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	sndTimer = V[Vx];
}

// Fx1E - ADD I, Vx
// Set I = I + Vx
void Chip8::_fx1e() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	I += V[Vx];
}

// Fx29 - LD F, Vx
// Set I = location of sprite for digit Vx
void Chip8::_fx29() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = V[Vx];

	I = MEM_FNT + (5 * digit);
}

// Fx33 - LD B, Vx
// Store BCD representation of Vx in memory locations I, I+1, and I+2
void Chip8::_fx33() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = V[Vx];

	// Ones-place
	memory[I + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[I + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[I] = value % 10;
}

// Fx55 - LD [I], Vx
// Store registers V0 through Vx in memory starting at location I
void Chip8::_fx55() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		memory[I + i] = V[i];
	}
}

// Fx65 - LD Vx, [I]
// Read registers V0 through Vx from memory starting at location I
void Chip8::_fx65() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i) {
		V[I] = memory[I + i];
	}
}

} // chip8 namespace
