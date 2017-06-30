#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "InstructionSet.h"

u_int8_t font_set[SIZE_FS] = {
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

void push(address addr)
{
	if (sp < (address*) STACK_UP) {
		printf("Stack overflow\n");
		exit(EXIT_FAILURE);
	}
	*(sp--) = addr;
}

address pop()
{
	if (sp == (address*) STACK_LOW) {
		printf("Empty stack\n");
		exit(EXIT_FAILURE);
	}
	return *(++sp);
}

void CLS()
{
	memset(screen, 0, WIDTH * HEIGHT);
}

void RET()
{
	PC = pop() + 2;
}

void JP(instruction i)
{
	PC = ADDR(i);
}

void CALL(instruction i)
{
	push(PC - 2);
	PC = ADDR(i);
}

void SE(instruction i)
{
	if (v[VX(i)] == BYTE(i)) {
		PC += 2;
	}
}

void SNEI(instruction i)
{
	if (v[VX(i)] != BYTE(i)) {
		PC += 2;
	}
}

void SR(instruction i)
{
	if (v[VX(i)] == v[VY(i)]) {
		PC += 2;
	}
}

void LDB(instruction i)
{
	v[VX(i)] = BYTE(i);
}

void ADDI(instruction i)
{
	v[VX(i)] += BYTE(i);
}

void LDR(instruction i)
{
	v[VX(i)] = v[VY(i)];
}

void OR(instruction i)
{
	v[VX(i)] |= v[VY(i)];
}

void AND(instruction i)
{
	v[VX(i)] &= v[VY(i)];
}

void XOR(instruction i)
{
	v[VX(i)] ^= v[VY(i)];
}

void ADD(instruction i)
{
	v[0xF] = v[VX(i)] > 0xFF - v[VY(i)] ? 1 : 0;
	v[VX(i)] += v[VY(i)];

}

void SUB(instruction i)
{
	v[0xF] = v[VY(i)] > v[VX(i)] ? 0 : 1;
	v[VX(i)] -= v[VY(i)];
}

void SHR(instruction i)
{
	v[0xF] = LSBI(v[VX(i)]);
	v[VX(i)] >>= 1;
}

void SUBN(instruction i)
{
	v[0xF] = v[VX(i)] > v[VY(i)] ? 0 : 1;
	v[VX(i)] = v[VY(i)] - v[VX(i)];
}

void SHL(instruction i)
{
	v[0xF] = MSBR(v[VX(i)]);
	v[VX(i)] <<= 1;
}

void SNE(instruction i)
{
	if (v[VX(i)] != v[VY(i)]) {
		PC += 2;
	}
}

void LDI(instruction i)
{
	I = ADDR(i);
}

void JPR(instruction i)
{
	PC = ADDR(i) + v[0x0];
}

void RND(instruction i)
{
	v[VX(i)] = (rand() % 256) & BYTE(i);
}

void DRW(instruction i)
{
	int x;
	int y;
	u_int8_t p;
	u_int8_t Vx = v[VX(i)];
	u_int8_t Vy = v[VY(i)];
	u_int8_t height = LSN(i);
	v[0xF] = 0;

	for (y = 0; y < height; y++) {
		p = RAM[I + y];
		for (x = 0; x < 8; x++) {
			if (p & (0x80 >> x)) {
				if (screen[x + Vx + (y + Vy) * WIDTH]) {
					v[0xF] = 1;
				}
				screen[x + Vx + (y + Vy) * WIDTH] ^= 1;
			}
		}
	}
}

void SKP(instruction i)
{
	if (keys[v[VX(i)]]) {
		PC += 2;
	}
}

void SKNP(instruction i)
{
	if (!keys[v[VX(i)]]) {
		PC += 2;
	}
}

void LDD(instruction i)
{
	v[VX(i)] = delay_timer;
}

void LDK(instruction i)
{
	for (int j = 0; j < NUM_REGS; j++) {
		if (keys[j]) {
			v[VX(i)] = j;
			return;
		}
	}
	/* Repeat instruction if no key pressed */
	PC -= 2;
}

void STD(instruction i)
{
	delay_timer = v[VX(i)];
}

void STS(instruction i)
{
	sound_timer = v[VX(i)];
}

void IINC(instruction i)
{
	v[0xF] = I + v[VX(i)] > 0xFFF ? 1 : 0;
	I += v[VX(i)];
}

void LDF(instruction i)
{
	I = v[VX(i)] * 5;
}

void BCD(instruction i)
{
	RAM[I] = v[VX(i)] / 100;
	RAM[I + 1] = (v[VX(i)] / 10) % 10;
	RAM[I + 2] = (v[VX(i)] % 100) % 10;
}

void STA(instruction i)
{
	for (int j = 0; j <= VX(i); j++) {
		RAM[I + j] = v[j];
	}
}

void LDA(instruction i)
{
	for (int j = 0; j <= VX(i); j++) {
		v[j] = RAM[I + j];
	}
}
