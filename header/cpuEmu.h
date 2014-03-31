#ifndef CPUEMU_H_INCLUDED
#define CPUEMU_H_INCLUDED

#include <Windows.h>
#include "fonts.h"
#include "typedefs.h"

#define numVAO 2
#define numVBO 2
#define numTexBuffers 2
#define numElemIndArray 1
#define numUnifHandles 1

#define SCREEN_WIDTH 64
#define SCREEN_HEIGTH 32

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
	// glut functions
	void display();
	void reshape_display(int w, int h);
	// display functions
	void setupTexture();
	void updateTexture();
	GLuint VAOHandles[numVAO];
	GLuint VBOHandles[numVBO];
	GLuint elemIndHandles[numElemIndArray];
	GLuint texBufferHandles[numTexBuffers];
	GLuint uniformBuffHandles[numUnifHandles];
	GLuint baseTex[SCREEN_WIDTH * SCREEN_HEIGTH * 4];
	GLfloat vertex[8] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};
	GLfloat texCoordinates[8] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	GLuint vertexIndex[6] = {
		0, 1, 2,
		2, 3, 0
	};
	GLuint pHandle;
	GLuint vHandle;
	GLuint fHandle;
	GLint vStatus, fStatus, pStatus;
	enum AttribIDs { vertexPos = 0, texCoord = 1 };
	void loadShaderProgram();
	uint16_t opcode;
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I;
	uint16_t pc;
	uint8_t gfx[SCREEN_WIDTH][SCREEN_HEIGTH][4];
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint16_t my_stack[16];
	uint16_t sp;
	uint8_t keyboard[16];
	uint8_t windowModifier;
}myChip8;

#endif // CPUEMU_H_INCLUDED
