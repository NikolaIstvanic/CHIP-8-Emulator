/*
 * Instruction Set for the CHIP-8 language.
 *
 * INFO:
 * https://en.wikipedia.org/wiki/CHIP-8
 *
 * CREATED:
 * 2017-06-21
 *
 * AUTHOR:
 * Nikola Istvanic
 */
#ifndef INSTRUCTIONSET_H_
#define INSTRUCTIONSET_H_

/* Space out the stack so that it can hold 16 2-byte addresses. Upper and lower
 * boundaries are separated by 30 because using zero-indexing, it can hold 16
 * 2-byte addresses.
 */
#define STACK_UP  0xEA0 // upper bound of the stack
#define STACK_LOW 0xEBE // lower bound of the stack
#define SIZE_MEM  4096  // number of bytes in memory
#define SIZE_FS   80    // size of the font-set
#define NUM_REGS  16    // number of registers
#define NUM_KEYS  16    // number of CHIP-8 input keys

#define WHITE  0
#define BLACK  0xFFFFFFFF
#define WIDTH  64  // width of CHIP-8 screen
#define HEIGHT 32  // height of CHIP-8 screen
#define EMU_W  640 // width of emulator screen
#define EMU_H  320 // height of emulator screen
#define BPP    32  // Bits Per Pixel on emulator screen

/* Create an instruction from two adjacent locations in memory */
#define INSTR(pc, pc_next) ((pc) << 8 | (pc_next))
/* Vx specifier from an instruction */
#define VX(instr) (((instr) & 0xF00) >> 8)
/* Vy specifier from an instruction */
#define VY(instr) (((instr) & 0xF0) >> 4)
/* Return an address from an instruction */
#define ADDR(instr) ((instr) & 0xFFF)
/* Least significant byte from an instruction */
#define BYTE(instr) ((instr) & 0xFF)
/* Least Significant Nibble from an instruction */
#define LSN(instr) ((instr) & 0xF)
/* Most Significant Nibble for instruction */
#define MSN(instr) (((instr) & 0xF000) >> 12)
/* Most Significant Bit for Register value */
#define MSBR(v) ((v) >> 7)
/* Least Significant Bit */
#define LSBI(v) ((v) & 0x1)

typedef unsigned short instruction; // instructions are 16-bit in granularity
typedef unsigned short address; // addresses for variables like PC are 16-bit
typedef unsigned short u_int16_t;
typedef unsigned char  u_int8_t;

/*
 * CHIP-8 memory consists of 4K (4096) locations. Memory is divided amongst:
 *     CHIP-8 interpreter   (0x000 - 0x1FF)
 *     Program in execution (0x200 - 0xE99)
 *     16 level stack       (0xEA0 - 0xEFF)
 *     Display refresh      (0xF00 - 0xFFF)
 */
u_int8_t RAM[SIZE_MEM];

/* Index (I) and program counter (PC) registers have 2 byte granularity */
u_int16_t PC;
u_int16_t I;

/* Font-set for the CHIP-8 */
u_int8_t font_set[SIZE_FS];

/* The CHIP-8 has 16 8-bit (1 byte) registers, labeled V0, V1, ..., VF */
u_int8_t v[NUM_REGS];

/* Input keys for the CHIP-8 emulator. Standard CHIP-8 hardware input is ordered
 * in the following way:
 *     1 2 3 C
 *     4 5 6 D
 *     7 8 9 E
 *     A 0 B F
 */
u_int8_t keys[NUM_KEYS];

/*
 * CHIP-8 has stack which stores address stored in PC before a subroutine call
 * which is then restored after the call. This emulator supports 16 levels
 * within the stack, defined as STACK_BOUND
 *
 * Stack pointer points to first empty location on the stack.
 */
address* sp;

/* Screen has 2K (2048) pixels (64 x 32). A pixel is either on (1) or off (0) */
u_int8_t screen[WIDTH * HEIGHT];

/* CHIP-8 has two timers which count down at 60 Hz to 0 from wherever set */
u_int8_t delay_timer;
/* CHIP-8 sound timer produces sound whenever it contains a non-zero value */
u_int8_t sound_timer;

/* PROTOTYPES */
/*
 * Push an address onto the stack if it's not full; move up the stack pointer.
 *
 * If the stack is full, the emulator will end.
 */
void push(address addr);

/*
 * Return which address is at the top of the stack; decrement the stack pointer.
 *
 * If the stack is empty, the emulator will end.
 */
address pop();

/*
 * Clear CHIP-8 screen.
 */
void CLS();

/*
 * Return from subroutine: set PC to address at the top of the stack + 2 so
 * whichever instruction was at PC doesn't get repeated, decrements stack
 * pointer.
 */
void RET();

/*
 * Jump to address. Instruction should have form INNN where NNN is the address
 * to jump to. Sets PC to NNN.
 */
void JP(instruction i);

/*
 * Call subroutine at address. Instruction should have form 2NNN where NNN is
 * the address of the subroutine.
 *
 * Pushes PC for this instruction onto the stack; sets PC equal to NNN.
 */
void CALL(instruction i);

/*
 * Skips next instruction if value held in register specified in instruction
 * equals value in instruction.
 */
void SE(instruction i);

/*
 * Skips next instruction if value held in register specified in instruction
 * does not equal immediate value specified in least significant byte of
 * instruction.
 */
void SNEI(instruction i);

/*
 * Skips next instruction if value held in register specified in instruction
 * equals value held in other register specified in instruction.
 */
void SR(instruction i);

/*
 * Load immediate byte value specified in instruction into register specified in
 * instruction.
 */
void LDB(instruction i);

/*
 * Add immediate value specified in instruction to register specified in
 * instruction.
 */
void ADDI(instruction i);

/*
 * Load value located in register specified in instruction to other register
 * specified in instruction.
 */
void LDR(instruction i);

/*
 * Bitwise OR value held in register Vx with value held in register Vy; store
 * the result in Vx.
 */
void OR(instruction i);

/*
 * Bitwise AND value stored in Vx with value stored in Vy; store result into Vx.
 */
void AND(instruction i);

/*
 * Bitwise XOR value in register Vx with value in Vy; store in Vx.
 */
void XOR(instruction i);

/*
 * Add value in Vy to value already stored in Vx. VF is set to 1 if there will
 * be overflow from the addition.
 */
void ADD(instruction i);

/*
 * Subtract value in Vy from value held in Vx. VF is set to 0 if Vy is greater
 * than Vx; 1 otherwise.
 */
void SUB(instruction i);

/*
 * Perform one unsigned right shift on the value in Vx; store in Vx. VF is set
 * to the least significant bit of Vx.
 */
void SHR(instruction i);

/*
 * Vx is set to Vx subtracted from Vy. VF is set to 0 if Vx is greater than Vy;
 * 1 otherwise.
 */
void SUBN(instruction i);

/*
 * Performs one left shift on the value in Vx; store value in Vx. VF is set to
 * the most significant bit in Vx.
 */
void SHL(instruction i);

/*
 * Skip next instruction if value in Vx does not equal value in Vy.
 */
void SNE(instruction i);

/*
 * Load into I variable immediate value stored in least significant three
 * nibbles of the instruction.
 */
void LDI(instruction i);

/*
 * Set PC to least significant three nibbles of instruction + value in V0.
 */
void JPR(instruction i);

/*
 * Generate a random integer from 0 to 255 inclusive and perform a bitwise AND
 * on the result with the least significant byte of the instruction; store in
 * Vx.
 */
void RND(instruction i);

/*
 * Draw sprite onto the CHIP-8 screen at location (Vx, Vy), set VF = collision.
 */
void DRW(instruction i);

/*
 * Skip the next instruction if the key specified by the value in register Vx is
 * currently pressed.
 */
void SKP(instruction i);

/*
 * Skip the next instruction if the key specified by the value in register Vx is
 * currently not pressed.
 */
void SKNP(instruction i);

/*
 * Value of delay_timer is placed in Vx
 */
void LDD(instruction i);

/*
 * Halt execution until a key is pressed, value of key is stored in Vx.
 */
void LDK(instruction i);

/*
 * Store Vx in delay_timer.
 */
void STD(instruction i);

/*
 * Store Vx in sound_timer.
 */
void STS(instruction i);

/*
 * Increment I register by value in Vx. VF is 1 if overflow; 0 otherwise.
 */
void IINC(instruction i);

/*
 * Load location of sprite in Vx into I. Value in Vx ranges from 0x0 to 0xF.
 * This method sets I to the location of that sprite. Each sprite has five
 * 8-bit values in memory, so the value in Vx is multiplied by five.
 */
void LDF(instruction i);

/*
 * Store BCD representation of value in Vx in memory locations I for hundreds
 * place, I + 1 for tens place, I + 2 for ones place.
 */
void BCD(instruction i);

/*
 * Store all register values from V0 to Vx in memory starting at address I.
 */
void STA(instruction i);

/*
 * Load all register values from V0 to Vx from memory starting at address I.
 */
void LDA(instruction i);

#endif
