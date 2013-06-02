#ifndef CPUEMU_H_INCLUDED
#define CPUEMU_H_INCLUDED

class chip8
{
    public:
        void initialize(); // set the inital state of the system
        void emulateCycle(); // emulate a cycle of the chip8 CPU
};

#endif // CPUEMU_H_INCLUDED
