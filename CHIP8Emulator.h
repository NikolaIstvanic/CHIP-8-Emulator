/*
 * CHIP-8 Programming Language Emulator.
 *
 * Emulator includes method to input, interpret, and run CHIP-8 source code.
 * CHIP-8 source should be saved in a .rom file which will be read by the
 * load_source method using the name of the ROM. If the file contains any errors
 * in syntax, the emulator will print which address the error occurred and the
 * instruction itself.
 *
 * This emulator runs off of the processor notion of Fetch, Decode, and Execute.
 * When an instruction is ready to be ran, it must first be fetched from what
 * value is in memory at the value in the register PC.
 *
 * The intent of this instruction is determined in the Decode stage. Instead of
 * a microcontrol unit in hardware, decoding is accomplished by a switch block
 * which determines what kind of operation is being performed and on what
 * operand(s).
 *
 * With this information, the emulator can execute the operation on whichever
 * registers required. After this step, the cycle returns to the Fetch state.
 * This process is repeated until there are no more instructions to fetch.
 *
 * INFO:
 * https://en.wikipedia.org/wiki/CHIP-8
 *
 * CREATED:
 * 2017-06-20
 *
 * AUTHOR:
 * Nikola Istvanic
 */
#ifndef CHIP8EMULATOR_H_
#define CHIP8EMULATOR_H_

/* INCLUDE */
#include "InstructionSet.h"

/* Signal to refresh the screen after it's been edited */
u_int8_t draw;

/* Emulator keys. A normal CHIP-8 keyboard would be in the following order:
 *     1 2 3 C
 *     4 5 6 D
 *     7 8 9 E
 *     A 0 B F
 *
 * This emulator has these keys arranged in the following order:
 *     1 2 3 4
 *     Q W E R
 *     A S D F
 *     Z X C V
 */
uint8_t emulator_keys[NUM_REGS];

/* PROTOTYPES */
/*
 * Initialize all necessary values to their appropriate starting values.
 *
 * All registers in v are set to zero, the screen is zeroed out, all keys are
 * set to zero (not pressed), RAM (including the stack) is cleared, the font set
 * is loaded into RAM, the random number generator is seeded, PC is initialized
 * to 0x200, I is set to zero, and the stack pointer is initialized to point to
 * the lower bound of the stack (0xEBE).
 */
void initialize();

/*
 * Fetch instruction located at address PC.
 *
 * Because instructions are 2 bytes long and memory is in bits, instructions
 * are in adjacent 8-bit memory locations which are bitwise ORed together to get
 * the complete instruction. PC must then be incremented by 2 to skip the second
 * half of the instruction fetched.
 */
instruction fetch();

/*
 * Decode and execute the instruction returned by fetch.
 *
 * The process of decoding (determining what kind of operation is required,
 * which registers are used, which constants are used) an instruction usually
 * involves indexing into a microcontroller/ROM unit. This process is emulated
 * by having the decode method which determines what operation (defined in
 * InstructionSet.h) the current instruction performs via switch statement which
 * is then executed.
 *
 * Unlike other instruction sets, the CHIP-8's does not feature instructions
 * with unique op-codes; rather the entire instruction can be thought of as the
 * op-code. Many instructions however feature the same most significant nibble,
 * so in order to decode instructions, this emulator uses a switch on the most
 * significant byte which breaks down most of the instructions. Any special cases
 * within that switch is handled either by an if statement or another switch in
 * order to determine the instruction's operation.
 *
 * If the instruction being decoded is not a member of the instruction set, the
 * decode method will display an error message and end the emulation.
 */
void execute(instruction i);

/*
 * Decrement the delay and sound counters by 1 if greater than 0.
 *
 * Whenever greater than 0, both of these timers should decrease by 1 at a rate
 * of 60 Hz. For the sound timer specifically, whenever its value is greater
 * than 0, a sound should be made. In this emulator, printf("\a") is used to
 * achieve this.
 */
void _decrement_timers();

/*
 * Helper method to set which of the CHIP-8 keys are pressed (1) and which are
 * not (0) based on which of the emulated (keyboard) keys are pressed.
 *
 * During normal execution, if a keyboard on the machine running this emulator
 * is pressed, then this method will update the appropriate value in the keys
 * pointer to reflect this change.
 */
void _set_keys();

/*
 * Clears the emulated CHIP-8 screen and draw how it should appear after a call
 * to the DRW instruction.
 *
 * Whenever the DRW instruction is executed, the draw flag is set to 1, and in
 * the run method, the pixels on the screen will be updated. It is possible to
 * write this method in such a way that it only updates the pixels on the
 * emulator screen which were affected by the DRW instruction instead of this
 * implementation which clears and redraws the entire screen, but since ROMs
 * written in the CHIP-8 language often draw and clear sprites on their own,
 * the flickering effect seen when running a Pong game on this emulator is
 * inevitable unless rewriting the ROM.
 */
void refresh_screen();

/*
 * Runs program in RAM.
 *
 * While there are instructions without error, the emulator runs whatever CHIP-8
 * source is located in its RAM. This method will first initiate the emulator
 * screen, then it will continually perform the fetch, decode, and execute cycle
 * while there are instructions to execute.
 *
 * This method will also update the CHIP-8 keyboard values using the emulator
 * keys. For every key on the emulator keyboard that is pressed, the
 * corresponding CHIP-8 key in the keys pointer will also be marked as pressed.
 * Any non-pressed emulator key will be marked as not pressed.
 *
 * Delay and sound timers will also be decremented at the end of one of the
 * cycles.
 */
void run();

/*
 * Loads the CHIP-8 instructions located in a chosen file into the emulator's
 * RAM which will then be executed.
 *
 * Prompts user to input the ROM name to emulate. This method searches the
 * current directory for any file whose name directly matches the input entered
 * by the user. User input must include the .rom extension in the entered ROM
 * name. If a file with this name exists, the emulator loads and executes it;
 * otherwise, an error message will display, and the program will end.
 */
void load_source();

/*
 * Print the contents of the stack.
 *
 * Method to debug if necessary. Prints the contents of the stack and where the
 * stack pointer is located from memory addresses 0xEBE to 0xEA0 - 1 to see if
 * the stack is full.
 */
void print_stack();

#endif
