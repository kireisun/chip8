#include "cpuEmu.h"
#include "typedefs.h"
#include "fonts.h"
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <iostream>

#define fontBase 0
#define BUFFER_OFFSET(offset) ((GLvoid*)(offset))

void chip8::loadGamePath(char *arg)
{
	if (strlen(arg) > MAX_PATH)
	{
		printf("\n%s", "Game path too long. Exiting...");
		exit(0);
	}
	char *csPath = new char[MAX_PATH];
	GetModuleFileNameA(NULL, csPath, MAX_PATH); 
	uint8_t uiFullPathSize = (strlen(csPath) - strlen("chip8.exe")) + 6 + strlen(arg);
	char *csFullPath = new char[uiFullPathSize + 1];
	strncpy(csFullPath, csPath, strlen(csPath) - strlen("chip8.exe"));
	strncat(csFullPath, "games\\", strlen("games\\"));
	strncat(csFullPath, arg, strlen(arg));
	strncpy(gamePath, csFullPath, strlen(csFullPath));
	delete csPath;
	delete csFullPath;
}

void chip8::initialize()
{

	// load game in memory

	FILE* Game;
	Game = fopen(gamePath, "rb");
	if (ferror(Game) != 0)
	{
		printf("\n%s", "No such file or directory. Exiting...");
		exit(0);
	}
	fseek(Game, 0, SEEK_END);
	rewind(Game);
	uint8_t sizeOfGame = ftell(Game);
	uint8_t charRead = fread(&memory[0x200], sizeof(uint8_t), sizeOfGame, Game);
	if (sizeOfGame != charRead)
	{
		printf("\n%s", "Error in loading game in emulated memory. Exiting...");
		exit(0);
	}

	pc = 0x200; // pc starts at 0x200
	opcode = 0;     // reset current opcode
	I = 0;     // reset index register
	sp = 0;     // reset stack pointer
	uint8_t* memory_p = &memory[fontBase]; //pointer to memory where the fonts are
	uint8_t* font_p = &font[0]; //pointer to array containing the fonts

	// clear register V0 - VF

	for (int i = 0; i < 16; ++i)
	{
		V[i] = 0;
	}

	// clear display

	for (int x = 0; x < SCREEN_WIDTH; ++x)
	{
		for (int y = 0; y < SCREEN_HEIGTH; ++y)
		{
			gfx[x][y][0] = gfx[x][y][1] = gfx[x][y][2] = 0;
			gfx[x][y][3] = 255;
		}
	}
	
	// clear stack

	for (int i = 0; i < 16; ++i)
	{
		my_stack[i] = 0;
	}

	// clear memory

	for (int i = 0; i < 4096; ++i)
	{
		memory[i] = 0;
	}

	// load fontset

	memcpy(memory_p, font_p, 5 * 16);

	// reset timers
}

void chip8::display()
{
	// emulation loop

	emulateCycle();

	// if the draw flag is set, eupdate the screen
	
	if (drawFlag)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		updateTexture();

		glutSwapBuffers();

		drawFlag = false;
	}
}

// create a black texture

void chip8::setupTexture()
{	
	glGenVertexArrays(1, &VAOHandles[0]);
	glBindVertexArray(VAOHandles[0]);

	glGenBuffers(1, &VBOHandles[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOHandles[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) + sizeof(texCoordinates), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex), vertex);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex), sizeof(texCoord), texCoordinates);

	glGenBuffers(1, elemIndHandles);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemIndHandles[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndex), vertexIndex, GL_STATIC_DRAW);

	glGenTextures(1, &texBufferHandles[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texBufferHandles[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SCREEN_WIDTH, SCREEN_HEIGTH);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,
		SCREEN_WIDTH, SCREEN_HEIGTH,
		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gfx);

	loadShaderProgram();

	uniformBuffHandles[0] = glGetUniformLocation(pHandle, "tex");
	glUniform1ui(uniformBuffHandles[0], 0);

	glVertexAttribPointer(vertexPos, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vertexPos);

	glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertex));
	glEnableVertexAttribArray(texCoord);

	glBindBuffer(GL_ARRAY_BUFFER, VBOHandles[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemIndHandles[0]);
	glBindTexture(GL_TEXTURE_2D, texBufferHandles[0]);

	glBindVertexArray(0);

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAOHandles[0]);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glutSwapBuffers();
}

void chip8::loadShaderProgram()
{
	// vertex and fragment shaders

	char* v = "#version 140\n\
	in vec4 position;\n\
	in vec2 texture;\n\
	out vec2 texture_out\n\
	void main()\n\
	{\n\
		gl_Position = position;\n\
		texture_out = texture;\n\
	}\n";

	char* f = "#version 140\n\
	uniform sampler2D tex;\n\
	in vec2 texture_out;\n\
	out vec4 color_out;\n\
	void main()\n\
	{\n\
		color_out = texture2D(tex, texture_out);\n\
	}";

	const char* vv = v;
	const char* ff = f;

	// compile shaders

	vHandle = glCreateShader(GL_VERTEX_SHADER);
	fHandle = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vHandle, 1, &vv, NULL);
	glShaderSource(fHandle, 1, &ff, NULL);

	glCompileShader(vHandle);
	glCompileShader(fHandle);

	try
	{
		glGetShaderiv(vHandle, GL_COMPILE_STATUS, &vStatus);


		if (GL_TRUE != vStatus)
		{
			throw vStatus;
		}

	}
	catch (GLint vCompilationError)
	{
		printf("Error in compiling vector shader\nLast OpenGL error is %i\nError log is:\n", vCompilationError);
		GLcharARB vErrorBuffer[512];
		glGetShaderInfoLog(vHandle, 512, NULL, vErrorBuffer);
		for (int i = 0; i < 512 && (vErrorBuffer[i] != '\0'); i++)
		{
			printf("%c", vErrorBuffer[i]);
		}
		printf("\n");
		exit(0);
	}

	try
	{
		glGetShaderiv(fHandle, GL_COMPILE_STATUS, &fStatus);

		if (GL_TRUE != fStatus)
		{
			throw fStatus;
		}
	}
	catch (GLint fCompilationError)
	{
		printf("Error in compiling fragment shader\nLast OpenGL error is %i\nError log is:\n", fCompilationError);
		GLcharARB vErrorBuffer[512];
		glGetShaderInfoLog(vHandle, 512, NULL, vErrorBuffer);
		for (int i = 0; i < 512 && (vErrorBuffer[i] != '\0'); i++)
		{
			printf("%c", vErrorBuffer[i]);
		}
		printf("\n");
		exit(0);
	}

	pHandle = glCreateProgram();

	glAttachShader(pHandle, vHandle);
	glAttachShader(pHandle, fHandle);

	glBindAttribLocation(pHandle, 0, "position");
	glBindAttribLocation(pHandle, 1, "texture");

	glLinkProgram(pHandle);

	try
	{
		glGetProgramiv(pHandle, GL_LINK_STATUS, &pStatus);

		if (GL_TRUE != pStatus)
		{
			throw pStatus;
		}
	}
	catch (GLint pCompilationStatus)
	{
		printf("Error in the linking phase\nLast OpenGL error is %i\nError log is:\n", pCompilationStatus);
		GLcharARB pErrorBuffer[512];
		glGetProgramInfoLog(pHandle, 512, NULL, pErrorBuffer);
		for (int i = 0; i < 512 && (pErrorBuffer[i] != '\0'); i++)
		{
			printf("%c", pErrorBuffer[i]);
		}
		printf("\n");
		exit(0);
	}

	glUseProgram(pHandle);
}


void chip8::emulateCycle()
{
	// fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// decode opcode

	switch (opcode & 0xF000) //select for the first 4 bits, for some opcodes there is the need for more checks
	{
	case 0x0000: // check instruction to execute
		switch (opcode & 0x000F)
		{
		case 0x0000: // 00E0: clear the screen
			for (int i = 0; i < 64; ++i)
			{
				for (int j = 0; j < 32; ++j)
				{
					gfx[i][j] = 0;
				}
			}
			drawFlag = true;
			pc += 2;
			break;
		case 0x000E: // 00EE: returns from a subroutine
			pc = my_stack[--sp];
			break;
		default:
			unknownOpcode();
		}
	case 0x1000: // 1NNN: jump to address NNN
		pc = opcode & 0x0FFF;
		break;
	case 0x2000: // 2NNN: call subroutine at NNN
		my_stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;
	case 0x3000: // 3XNN: compare VX with NN and if they match skips the next instruction
		if (V[opcode & 0x0F00] == opcode & 0x00FF)
		{
			pc += 4;
			break;
		}
		else
			pc += 2;
		break;
	case 0x4000: // 4XNN: skips the next instruction if VX != NN
		if (V[opcode & 0x0F00] != opcode & 0x00FF)
		{
			pc += 4;
			break;
		}
		else
			pc += 2;
		break;
	case 0x5000: // 5XY0: skips the next instruction if VX == VY
		if (V[opcode & 0x0F00] == V[opcode & 0x00F0])
		{
			pc += 4;
			break;
		}
		else
			pc += 2;
		break;
	case 0x6000: // 6XNN: sets VX == NN
		V[opcode & 0x0F00] = opcode & 0x00FF;
		pc += 2;
		break;
	case 0x7000: // 7XNN: adds NN to VX
		V[opcode & 0x0F00] += V[opcode & 0x00FF];
		pc += 2;
		break;
	case 0x8000: // check instruction to execute
		switch (opcode & 0x000F)
		{
		case 0x0000: // 8XY0: sets VX = VY
			V[opcode & 0x0F00] = V[opcode & 0x00F0];
			pc += 2;
			break;
		case 0x0001: // 8XY1: VX = VX OR VY
			V[opcode & 0x0F00] = V[opcode & 0x0F00] | V[opcode & 0x00F0];
			pc += 2;
			break;
		case 0x0002: // 8XY2: VX = VX AND VY
			V[opcode & 0x0F00] = V[opcode & 0x0F00] & V[opcode & 0x00F0];
			pc += 2;
			break;
		case 0x0003: // 8XY3: VX = VX XOR VY
			V[opcode & 0x0F00] = V[opcode & 0x0F00] ^ V[opcode & 0x00F0];
			pc += 2;
			break;
		case 0x0004: // 8XY4: VX = VX + VY, VF = 1 if sum is greater than 8 bits
			if (V[opcode & 0x0F00] + V[opcode & 0x00F0] > 0x00FF)
			{
				V[opcode & 0x0F00] = (V[opcode & 0x0F00] + V[opcode & 0x00F0]) & 0x000F;
				V[15] = 1;
				pc += 2;
				break;
			}
			else
			{
				V[opcode & 0x0F00] = V[opcode & 0x0F00] + V[opcode & 0x00F0];
				V[15] = 0;
				pc += 2;
				break;
			}
		case 0x0005: //8XY5: VX = VX - VY, VF = NOT borrow
			if (V[opcode & 0x0F00] - V[opcode & 0x00F0] >= 0)
			{
				V[opcode & 0x0F00] = V[opcode & 0x0F00] - V[opcode & 0x00F0];
				V[15] = 1;
				pc += 2;
				break;
			}
			else
			{
				V[opcode & 0x0F00] = V[opcode & 0x00F0] - V[opcode & 0x0F00];
				V[15] = 0;
				pc += 2;
				break;
			}
		case 0x0006: // 8XY6: SHR VX >> 1, VF = Vx in least significant bit
			V[15] = (V[opcode & 0x0F00] & 0x1);
			V[opcode & 0x0F00] >> 1;
			pc += 2;
		case 0x0007: // 8XY7: VX = VY - VX, VF = NOT borrow
			if (V[opcode & 0x00F0] - V[opcode & 0x0F00] >= 0)
			{
				V[opcode & 0x0F00] = V[opcode & 0x00F0] - V[opcode & 0x0F00];
				V[15] = 1;
				pc += 2;
				break;
			}
			else
			{
				V[opcode & 0x0F00] = V[opcode & 0x0F00] - V[opcode & 0x00F0];
				V[15] = 0;
				pc += 2;
				break;
			}
		case 0x000E: // 8XYE: SHR VX << 1, VF = Vx in most significant bit
			V[15] = (V[opcode & 0x0F00] & 0x1);
			V[opcode & 0x0F00] << 1;
			pc += 2;
		default:
			unknownOpcode();
		}
	case 0x9000: // 9XY0: skips next instruction if VX != VY
		if (V[opcode & 0x0F00] != V[opcode & 0x00F0])
		{
			pc += 4;
			break;
		}
		pc += 2;
		break;
	case 0xA000: // ANNN: sets I to the address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	case 0xB000: // BNNN: jump to address NNN + V0
		my_stack[sp] = pc;
		++sp;
		pc = (opcode & 0x0FFF + V[0]);
		break;
	case 0xC000: // CXNN: sets VX = random byte AND NN
		srand(time(NULL));
		V[opcode & 0x0F00] = V[opcode & 0x00FF] && (rand() % 255);
		pc += 2;
		break;
	case 0xD000: // DXYN: graphics
		uint16_t X = V[opcode & 0x0F00];
		uint16_t Y = V[opcode & 0x00F0];
		uint16_t height = V[opcode & 0x000F];
		uint16_t Ipixel;
		V[0xF] = 0;
		for (int yRow = 0; yRow < height; ++yRow)
		{
			Ipixel = memory[I + yRow];
			for (int xRow = 0; xRow < 8; ++xRow)
			{
				if (Ipixel & (0x80 >> xRow) != 0)
				{
					if (gfx[X + xRow + ((Y + yRow) * 64)] == 1)
					{
						V[0xF] = 1;
					}
					gfx[X + xRow + ((Y + yRow) * 64)] ^= 1;
				}
			}
		}
		pc += 2;
		drawFlag = true;
		break;
	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E: //EX9E: skips instruction if key in VX is pressed
			if (key[V[opcode & 0x0F00] != 0)
			{
				pc += 4;
			}
			else
				pc += 2;
			break;
		case 0x00A1: // EXA1: skips instruction if key in VX is not pressed
			if (key[V[opcode & 0x0F00] != 0)
			{
				pc += 2;
			}
			else
				pc += 4;
			break;
		default:
			unknownOpcode();
		}
	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FXA1: Sets VX to the value of the delay timer
			V[opcode & 0x0F00] = delay_timer;
			pc += 2;
			break;
		case 0x000A: // FX0A: Awaits a key press and then stores it in VX
			bool keynotPressed = true;
			while (keynotPressed)
			{
				for (int i = 0; i < 16; i++)
				{
					if (key[i] != 0)
					{
						keynotPressed = false;
						break;
					}
				}
			}
			pc += 2;
			break;
		case 0x0015: // FX15: Sets the delay timer to VX
			delay_timer = V[opcode & 0x0F00];
			pc += 2;
			break;
		case 0x0018: // FX18: Sets the sound timer to VX
			sound_timer = V[opcode & 0x0F00];
			pc += 2;
			break;
		case 0x001E: // FX1E: Adds VX to I
			I += V[opcode & 0x0F00];
			pc += 2;
			break;
		case 0x0029: // FX29: Set I to the location of sprite for digit VX
			I = fontBase + (V[opcode & 0x0F00] * 5);
			pc += 2;
			break;
		case 0x0033: // FX33: store BCD representation of VX in memory locations I, I+1, and I+2
				uint8_t storeValue = V[opcode & 0x0F00];
				uint8_t hundreds, tens, ones;
				ones = storeValue % 10;
				storeValue /= 10;
				tens = storeValue % 10;
				hundreds = storeValue / 10;
				memory[I] = hundreds;
				memory[I + 1] = tens;
				memory[I + 2] = ones;
				pc += 2;
				break;
			case 0x0055: // FX55: store registers V0 - VX in memory starting at address I
				int y = 0;
				for (int i = 0; i < 16; ++i, ++y)
				{
					memory[I + y] = V[i];
				}
				pc += 2;
				break;
			case 0x0065: // FX65: fill registers V0 - VX with values starting at address I
				int y = 0;
				for (int i = 0; i < 16; ++i, ++y)
				{
					V[i] = memory[I + y];
				}
				pc += 2;
				break;
			default:
				unknownOpcode();
		}
	default:
		unknownOpcode();
	}

	// update timers
}
