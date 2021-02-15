namespace chip8 {
#define MAX_MEM 4096
#define NUM_REGS 16
#define FONTS_SIZE 80
#define STACK_SIZE 48
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define MAX_KEYS 16

// These memory offsets are to replicate reserved memory blocks iin original
// CHIP-8 emulators
#define MEM_LO 0x200
#define MEM_HI 0xFFF
#define MEM_FNT 0x50
} //chip8 namespace