
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
        case 0x3000: // 3XNN: compare Vx with NN and if they match skips the next instruction
            if(V[opcode & 0x0F00] == opcode & 0x00FF)
            {
                pc += 4;
                break;
            }
            else
            pc += 2;
            break;
        case 0x4000: // 4XNN: skips the next instruction if Vx != NN
            if(V[opcode & 0x0F00] != opcode & 0x00FF)
            {
                pc += 4;
                break;
            }
            else
            pc += 2;
            break;
        case 0x5000: // 5XY0: skips the next instruction if Vx == Vy
            if(V[opcode & 0x0F00] == V[opcode & 0x00F0])
            {
                pc += 4;
                break;
            }
            else
            pc += 2;
            break;
        case 0x6000: // 6XNN: sets Vx == NN
            V[opcode & 0x0F00] = opcode & 0x00FF;
            pc += 2;
            break;
        case 0x7000: // 7XNN: adds NN to Vx
            V[opcode & 0x0F00] += V[opcode & 0x00FF];
            pc += 2;
            break;
        case 0x8000: // check instruction to execute
            switch(opcode & 0x000F)
            {
                case 0x0000: // 8XY0: sets Vx = Vy
                    V[opcode & 0x0F00] = V[opcode & 0x00F0];
                    pc += 2;
                    break;
                case 0x0001: // 8XY1: Vx = Vx OR Vy
                    V[opcode & 0x0F00] = V[opcode & 0x0F00] | V[opcode & 0x00F0];
                    pc += 2;
                    break;
                case 0x0002: // 8XY2: Vx = Vx AND Vy
                    V[opcode & 0x0F00] = V[opcode & 0x0F00] & V[opcode & 0x00F0];
                    pc += 2;
                    break;
                case 0x0003: // 8XY3: Vx = Vx XOR Vy
                    V[opcode & 0x0F00] = V[opcode & 0x0F00] ^ V[opcode & 0x00F0];
                    pc += 2;
                    break;
                case 0x0004: // 8XY4: Vx = Vx + Vy, VF = 1 if sum is greater than 8 bits
                    if(V[opcode & 0x0F00] + V[opcode & 0x00F0] > 0x00FF)
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
                case 0x0005: //8XY5: Vx = Vx - Vy, VF = NOT borrow
                    if(V[opcode & 0x0F00] - V[opcode & 0x00F0] >= 0)
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
                case 0x0006: // 8XY6: SHR Vx >> 1, VF = Vx in least significant bit
                    V[15] = (V[opcode & 0x0F00] & 0x1);
                    V[opcode & 0x0F00] >> 1;
                    pc += 2;
                case 0x0007: // 8XY7: Vx = Vy - Vx, VF = NOT borrow
                    if(V[opcode & 0x00F0] - V[opcode & 0x0F00] >= 0)
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
                case 0x000E: // 8XYE: SHR Vx << 1, VF = Vx in most significant bit
                    V[15] = (V[opcode & 0x0F00] & 0x1);
                    V[opcode & 0x0F00] << 1;
                    pc += 2;
                default:
                    unknownOpcode();
            }
        case 0x9000: // 9XY0: skips next instruction if VX != VY
            if(V[opcode & 0x0F00] != V[opcode & 0x00F0])
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
        case 0xC000: // CXNN: sets Vx = random byte AND NN
            srand(time(NULL));
            V[opcode & 0x0F00] = V[opcode & 0x00FF] && (rand() % 255);
            pc += 2;
            break;
        case 0xD000: // DXYN: graphics
            unsigned short X = V[opcode & 0x0F00];
            unsigned short Y = V[opcode & 0x00F0];
            unsigned short height = V[opcode & 0x000F];
            unsigned short Ipixel;
            V[0xF] = 0;
            for(int yRow = 0; yRow < height; ++yRow)
            {
                Ipixel = memory[I + yRow];
                for(int xRow = 0; xRow < 8; ++xRow)
                {
                    if(Ipixel & (0x80 >> xRow) != 0)
                    {
                        if(gfx[X + xRow + ((Y + yRow) * 64)] == 1)
                        {
                            V[0xF] = 1;
                        }
                        gfx[X + xRow + ((Y + yRow) * 64)] ^= 1;
                    }
                }
            }
            pc += 2;
            break;
        case 0xE000:
            switch(opcode & 0x00FF)
            {
                case 0x009E: //EX9E: skips instruction if key in VX is pressed
                    if(key[V[opcode & 0x0F00] != 0)
                    {
                        pc += 4;
                    }
                    else
                        pc += 2;
                    break;
                case 0x00A1: // EXA1: skips instruction if key in VX is not pressed
                    if(key[V[opcode & 0x0F00] != 0)
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
            switch(opcode & 0x00FF)
            {
                case 0x0007: // FXA1: Sets VX to the value of the delay timer
                    V[opcode & 0x0F00] = delay_timer;
                    pc += 2;
                    break;
                case 0x000A: // FX0A: Awaits a key press and then stores it in VX
                    bool keynotPressed = true;
                    while(keynotPressed)
                    {
                        for(int i = 0; i < 16; i++)
                        {
                            if(key[i] != 0)
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

            }
        default:
            unknownOpcode();
    }

    // update timers
}
