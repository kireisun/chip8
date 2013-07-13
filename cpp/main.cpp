//#include OpenGL graphics and input libraries
#include "cpuEmu.h" // my CPU core implementation
#include "fonts.h" // include fonts
#include <ctime>
#include <cstdlib>

chip8 myChip8;

int main(int argc, char **argv)
{
    uint16_t opcode;
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t gfx[64 * 32];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t my_stack[16];
    uint16_t sp;
    uint8_t keyboard[16];

    // set up render system and register input callbacks

    setupGraphics();
    setupInput();

    // initialize the chip8 system and load the game into memory
    myChip8.initialize()
    myChip8.loadGame()

    // emulation loop
    for(;;)
    {
        // emulate one cycle
        myChip8.emulateCycle();

        // if the draw flag is set, eupdate the screen
        if(myChip8.drawFlag)
            drawGraphics();

        // store key press state (press and release)
        myChip8.setKeys();
    }

    return 0;
}

