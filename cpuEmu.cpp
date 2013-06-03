
void chip8::initialize()
{
    pc     = 0x200; // pc starts at 0x200
    opcode = 0;     // reset current opcode
    I      = 0;     // reset index register
    sp     = 0;     // reset stack pointer

    // clear register V0 - VF

    for(int i = 0; i < 16; ++i)
    {
        V[i] = 0;
    }

    // clear display

    for(int i = 0; i < 64; ++i)
    {
        for(int j = 0; j < 32; ++j)
        {
            gfx[i][j] = 0;
        }
    }

    // clear stack

    for(int i = 0; i < 16; ++i)
    {
        my_stack[i] = 0;
    }

    // clear memory

    for(int i = 0; i < 4096; ++i)
    {
        memory[i] = 0;
    }
}
    // load fontset
    // reset timers
void chip8::emulateCycle()
{
    // fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // decode opcode

    switch(opcode & 0xF000) //select for the first 4 bits, for some opcodes there is the need for more checks
    {
        case 0x0000: // check instruction to execute
            switch(opcode & 0x000F)
            {
                case 0x0000: // 00E0: clear the screen
                    for(int i = 0; i < 64; ++i)
                    {
                        for(int j = 0; j < 32; ++j)
                        {
                            gfx[i][j] = 0;
                        }
                    }
                    break;
                case 0x000E: // 00EE: returns from a subroutine
                    pc = my_stack[--sp];
                    break;
            }
        case 0x1000: // 1NNN: jump to address NNN
            pc = opcode & 0x0FFF;
            break;
        case 0x2000: // 2NNN: call subroutine at NNN
            my_stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000: // 3XNN: compare Vx with NN and if they match skips the next instruction
            if(V[opcode & 0x0F00] == opcode & 0x00FF)
            {
                pc = pc + 4;
                break;
            }
            else
            pc = pc + 2;
            break;
        case 0xA000: // ANNN: sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        default:
            cout << "Unknown opcode"; // needs something better
    }

    // update timers
}
