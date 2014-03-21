#ifndef CPUEMU_H_INCLUDED
#define CPUEMU_H_INCLUDED

#include <Windows.h>

class chip8
{
public:
	// set the inital state of the system
	void initialize(); 
	// emulate one cycle of the chip8 CPU
	void emulateCycle();
	// handles unknown opcodes
	void unknownOpcode();
	// indicates to update the screen
	bool drawFlag;
	// load the game path in the emulator
	void loadGamePath(char *arg);
	char gamePath[MAX_PATH];
	uint16_t opcode;
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I;
	uint16_t pc;
	uint8_t gfx[64][32];
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint16_t my_stack[16];
	uint16_t sp;
	uint8_t keyboard[16];
};

#endif // CPUEMU_H_INCLUDED
