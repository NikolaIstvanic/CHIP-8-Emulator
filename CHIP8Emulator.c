#include <SDL/SDL.h>
#include <time.h>
#include <unistd.h>
#include "CHIP8Emulator.h"

u_int8_t emulator_keys[NUM_KEYS] = {
	SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a,
	SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

void initialize()
{
	/* Initialize variables */
	PC = 0x200;
	I  = 0;
	sp = (address*) &RAM[STACK_LOW];

	/* Clear registers V0 - VF */
	memset(v, 0, NUM_REGS);

	/* Clear keys */
	memset(keys, 0, NUM_KEYS);

	/* Clear screen */
	CLS();

	/* Clear memory and calling stack */
	memset(RAM, 0, SIZE_MEM);

	/* Load font set */
	memcpy(RAM, font_set, SIZE_FS);

	/* Seed rand */
	srand(time(NULL));

	/* Clear timers */
	delay_timer = 0;
	sound_timer = 0;

	/* Clear draw flag */
	draw = 0;
}

instruction fetch()
{
	instruction i = INSTR(RAM[PC], RAM[PC + 1]);
	PC += 2;
	return i;
}

void execute(instruction i)
{
	/* Most Significant Nibble can be used to decode most instructions */
	u_int8_t msn = MSN(i);
	u_int8_t lsn = LSN(i);

	printf("Executing 0x%04X at PC = 0x%04X, I = 0x%04X\n", i, PC - 2, I);
	/* Decode and execute */
	switch (msn) {
		case 0x0:
			if (i == 0x00E0) {
				// 00E0 CLS: clear screen
				CLS();
				return;
			}
			if (i == 0x00EE) {
				// 00EE RET: return from subroutine
				RET();
				return;
			}
			break;
		case 0x1:
			// 1nnn JP: PC = nnn
			JP(i);
			return;
		case 0x2:
			// 2nnn CALL: push PC on stack, set PC to nnn
			CALL(i);
			return;
		case 0x3:
			// 3xkk SE: skip next instruction if Vx = kk
			SE(i);
			return;
		case 0x4:
			// 4xkk SNEI: skip next instruction if Vx != kk
			SNEI(i);
			return;
		case 0x5:
			// Least Significant Nibble must be 0
			if (!lsn) {
				// 5xy0 SR: skip next instruction if Vx = Vy
				SR(i);
				return;
			}
			break;
		case 0x6:
			// 6xkk LDB: load Vx with kk
			LDB(i);
			return;
		case 0x7:
			// 7xkk ADDI: Vx += kk
			ADDI(i);
			return;
		case 0x8:
			switch (lsn) {
				case 0x0:
					// 8xy0 LDR: load Vy into Vx
					LDR(i);
					return;
				case 0x1:
					// 8xy1 OR: Vx |= Vy
					OR(i);
					return;
				case 0x2:
					// 8xy2 AND: Vx &= Vy
					AND(i);
					return;
				case 0x3:
					// 8xy3 XOR: Vx ^= Vy
					XOR(i);
					return;
				case 0x4:
					// 8xy4 ADD: Vx += Vy; VF = 1 if overflow; VF = 0 otherwise
					ADD(i);
					return;
				case 0x5:
					// 8xy5 SUB: Vx -= Vy; VF = 1 if Vx > Vy; VF = 0 otherwise
					SUB(i);
					return;
				case 0x6:
					// 8xy6 SHR: Vx >>= 1; VF = Least Significant Bit of Vx
					SHR(i);
					return;
				case 0x7:
					/*
					 * 8xy7 SUBN: Vx = Vy - Vx; VF = 1 if Vy > Vx; VF = 0
					 * otherwise
					 */
					SUBN(i);
					return;
				case 0xE:
					// 8xyE SHL: Vx <<= 1; VF = Most Significant Bit of Vx
					SHL(i);
					return;
			}
			break;
		case 0x9:
			// Least Significant Nibble must be 0
			if (!lsn) {
				// 9xy0 SNE: skip next instruction if Vx != Vy
				SNE(i);
				return;
			}
			break;
		case 0xA:
			// Annn LDI: I = nnn
			LDI(i);
			return;
		case 0xB:
			// Bnnn JPR: PC = nnn + V0
			JPR(i);
			return;
		case 0xC:
			/*
			 * Cxkk RND: generate random number from 0 to 255, AND with kk;
			 * store in Vx
			 */
			RND(i);
			return;
		case 0xD:
			/*
			 * Dxyn DRW: draw n-byte sprite to the screen at memory address I at
			 * (Vx, Vy); set VF = collision. Draw flag is set to 1 to signal to
			 * the run method to refresh the screen
			 */
			DRW(i);
			draw = 1;
			return;
		case 0xE:
			if (BYTE(i) == 0x9E) {
				/*
				 * Ex9E SKP: skip next instruction if CHIP-8 input key with the
				 * value of Vx is pressed
				 */
				SKP(i);
				return;
			}
			if (BYTE(i) == 0xA1) {
				/*
				 * ExA1 SKNP: skip next instruction if CHIP-8 input key with the
				 * value of Vx is not pressed
				 */
				SKNP(i);
				return;
			}
			break;
		case 0xF:
			switch (BYTE(i)) {
				case 0x07:
					// Fx07 LDD: Vx = delay_timer
					LDD(i);
					return;
				case 0x0A:
					/*
					 * Fx0A LDK: wait for a CHIP-8 input key press, store the
					 * value of the key (0x0 - 0xF) in Vx
					 */
					LDK(i);
					return;
				case 0x15:
					// Fx15 STD: delay_timer = Vx
					STD(i);
					return;
				case 0x18:
					// Fx18 STS: sound_timer = Vx
					STS(i);
					return;
				case 0x1E:
					// Fx1E IINC: I += Vx
					IINC(i);
					return;
				case 0x29:
					// Fx29 LDF: I = location of sprite for value in Vx
					LDF(i);
					return;
				case 0x33:
					/*
					 * Fx33 BCD: store Binary Coded Decimal of value in Vx
					 * starting at memory address I for hundreds place, I + 1
					 * for tens, I + 2 for ones
					 */
					BCD(i);
					return;
				case 0x55:
					/*
					 * Fx55 STA: store values in registers V0 - Vx in memory,
					 * starting at address I
					 */
					STA(i);
					return;
				case 0x65:
					/*
					 * Fx65 LDA: load values for registers V0 - Vx from memory,
					 * starting at address I
					 */
					LDA(i);
					return;
				break;
			}
	}
	printf("Unknown instruction at PC = 0x%04X\n0x%04X", PC - 2, i);
	exit(EXIT_FAILURE);
}

void _decrement_timers()
{
    if (delay_timer > 0) {
        delay_timer--;
    }
    if (sound_timer > 0) {
    	if (sound_timer == 1) {
    		printf("\a");
    	}
        sound_timer--;
    }
}

void _set_keys()
{
	int i;
    u_int8_t* pressed = SDL_GetKeyState(NULL);

    if (pressed[SDLK_ESCAPE]) {
        exit(EXIT_SUCCESS);
    }
    for (i = 0; i < NUM_KEYS; i++) {
    	keys[i] = pressed[emulator_keys[i]];
    }
}

void refresh_screen()
{
	int x, y;
	SDL_Surface* emulator_screen = SDL_GetVideoSurface();

	/* Secure surface to access pixels */
	SDL_LockSurface(emulator_screen);
	Uint32* emulator_pixels = (Uint32*) emulator_screen->pixels;

	/* Clear emulator screen */
	memset(emulator_pixels, 0, EMU_W * EMU_H);

	/* Redraw emulator screen after DRW updates the CHIP-8 screen */
	for (x = 0; x < EMU_W; x++) {
		for (y = 0; y < EMU_H; y++) {
			emulator_pixels[x + y * EMU_W]
				= screen[x / 10 + (y / 10) * WIDTH] ? BLACK : WHITE;
		}
	}

	/* Release surface */
	SDL_UnlockSurface(emulator_screen);
	SDL_Flip(emulator_screen);
	SDL_Delay(10);
}

void run()
{
	int i;
	SDL_Event e;

	/* Initialize emulator screen */
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetVideoMode(EMU_W, EMU_H, BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);

	while (1) {
		if (SDL_PollEvent(&e)) {
			continue;
		}
		for (i = 0; i < 20; i++) {
			_set_keys();
			execute(fetch());
			if (draw) {
				draw = 0;
				refresh_screen();
			}
		}
		_decrement_timers();
	}
}

void load_source()
{
	char rom_name[50];
	FILE* rom;

	printf("\nEnter name of the CHIP-8 ROM (ending with .rom) to emulate: ");
	if (!scanf("%s", rom_name)) {
		printf("Error reading STDIN\n");
		fflush(stdin);
		exit(EXIT_FAILURE);
	}
	rom = fopen(rom_name, "rb");
	if (!rom)  {
		printf("ROM not found\n");
		fflush(stdin);
		exit(EXIT_FAILURE);
	}
	/*
	 * Read into RAM starting at 0x200, 1 byte at a time, maximum number of
	 * elements is 0xCA0 which is the number of bytes between 0x200 and where
	 * the stack begins
	 */
	if (!fread(RAM + 0x200, 1, 0xCA0, rom)) {
		printf("Error reading ROM\n");
		exit(EXIT_FAILURE);
	}
}

void print_stack()
{
	u_int8_t found = 0;

	/* check one memory address above the stack to see if it's full */
	printf("|         |");
	address* cp = (address*) &RAM[STACK_UP - sizeof(address)];
	if (cp == sp) {
		found = 1;
		printf(" <- sp");
	}
	printf("\n");
	/* print stack memory addresses */
	for (cp += 1; cp <= (address*) &RAM[STACK_LOW]; cp++) {
		printf("| 0x%04X |", *cp);
		if (!found && cp == sp) {
			found = 1;
			printf(" <- sp");
		}
		printf("\n");
	}
}

int main()
{
	initialize();
	load_source();
	run();
	return 0;
}
