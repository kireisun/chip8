#ifndef CPUEMU_H_INCLUDED
#define CPUEMU_H_INCLUDED

class chip8
{
    public:
        void initialize(); // set the inital state of the system
        void emulateCycle(); // emulate one cycle of the chip8 CPU
        void unknownOpcode(); // handles unknown opcodes
        bool drawFlag; // indicates to update the screen
};

#endif // CPUEMU_H_INCLUDED
